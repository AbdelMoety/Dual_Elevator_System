/** App_Slave.c */
#include "App_Internal.h"
#include "Usart.h"
#include "Spi.h"

static volatile uint16 MasterAgeMs = COMM_TIMEOUT_MS + 1U;
static volatile uint8 IndependentMode = 1U;

void Role_Init(void) {
    Spi1_SlaveInit(Spi_DoneCallback);
    BuildLocalPacket(0U, IPC_FLAG_INDEPENDENT, IpcTxFrame);
    Spi1_SlaveSetTxFrame(IpcTxFrame, SPI_FRAME_LEN);
    Usart1_TransmitString("SLAVE Board B ready\r\n");
}

void Role_TimeTick(void) {
    if (MasterAgeMs < 60000U) MasterAgeMs++;
}

void Role_Run(void) {
    if (Flag_SpiPeriod != 0U) {
        Flag_SpiPeriod = 0U;
        IndependentMode = (MasterAgeMs > COMM_TIMEOUT_MS) ? 1U : 0U;
        uint8 flag = IndependentMode ? IPC_FLAG_INDEPENDENT : 0U;
        BuildLocalPacket(0U, flag, IpcTxFrame);
        Spi1_SlaveSetTxFrame(IpcTxFrame, SPI_FRAME_LEN);
    }
    if (Flag_SpiDone != 0U) {
        Flag_SpiDone = 0U;
        if (Spi1_SlaveReadRxFrame(IpcRxFrame, SPI_FRAME_LEN) == OK) {
            IpcFrameType rx;
            if (Protocol_Parse(IpcRxFrame, &rx) == OK) {
                MasterAgeMs = 0U;
                IndependentMode = 0U;
                if (rx.cmd_mask != 0U) {
                    uint8 floor;
                    for (floor = FLOOR_MIN; floor <= FLOOR_MAX; floor++) {
                        if ((rx.cmd_mask & FloorToMask(floor)) != 0U) Elevator_AddTarget(&SelfElevator, floor);
                    }
                }
            }
        }
    }
}

void Role_Telemetry(void) {
    if (Flag_Telemetry == 0U) return;
    Flag_Telemetry = 0U;
    Usart1_TransmitString("S B:F"); Usart1_TransmitUnsigned(SelfElevator.current_floor);
    Usart1_TransmitString(" S:"); Usart1_TransmitString(Elevator_StateName(SelfElevator.state));
    Usart1_TransmitString(" T:0x"); Usart1_TransmitUnsigned(SelfElevator.target_mask);
    Usart1_TransmitString(" Mode:"); Usart1_TransmitString(IndependentMode ? "INDEP\r\n" : "LINK\r\n");
}