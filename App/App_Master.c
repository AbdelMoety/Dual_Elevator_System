/** App_Master.c */
#include "App_Internal.h"
#include "Usart.h"
#include "Spi.h"
#include "Gpio.h"
#include "Exti.h"
#include "Critical.h"

#define PORT_INPUTS GPIO_D
#define HALL_U1_LINE 5U
#define HALL_D2_LINE 6U
#define HALL_U2_LINE 7U
#define HALL_D3_LINE 8U
#define HALL_U3_LINE 9U
#define HALL_D4_LINE 10U

static volatile uint8 HallPending = 0U;
static volatile uint16 SlaveAgeMs = COMM_TIMEOUT_MS + 1U;
static IpcFrameType SlaveStatus = {0U, ELEVATOR_IDLE, 1U, 0U, IPC_FLAG_COMM_FAULT, 0U};
static volatile uint8 SlaveCmdMask = 0U;

static void HallU1_Callback(void) { HallPending |= (1U << 0U); }
static void HallD2_Callback(void) { HallPending |= (1U << 1U); }
static void HallU2_Callback(void) { HallPending |= (1U << 2U); }
static void HallD3_Callback(void) { HallPending |= (1U << 3U); }
static void HallU3_Callback(void) { HallPending |= (1U << 4U); }
static void HallD4_Callback(void) { HallPending |= (1U << 5U); }

static uint8 IsSlaveCommFault(void) { return (SlaveAgeMs > COMM_TIMEOUT_MS) ? 1U : 0U; }
static uint8 CallFloor(uint8 HallIndex) { static const uint8 floor[6] = {1U, 2U, 2U, 3U, 3U, 4U}; return floor[HallIndex]; }
static uint8 CallDir(uint8 HallIndex) { static const uint8 dir[6] = {DIR_UP, DIR_DOWN, DIR_UP, DIR_DOWN, DIR_UP, DIR_DOWN}; return dir[HallIndex]; }

static uint16 ScoreElevator(uint8 State, uint8 Floor, uint8 Emergency, uint8 CallFloorValue, uint8 CallDirection) {
    uint8 distance = (Floor > CallFloorValue) ? (uint8)(Floor - CallFloorValue) : (uint8)(CallFloorValue - Floor);
    if (Emergency != 0U) return 1000U;
    if (State == ELEVATOR_IDLE && Floor == CallFloorValue) return 0U; 
    if (State == ELEVATOR_MOVING_UP) return (CallDirection == DIR_UP) ? ((CallFloorValue > Floor) ? 10U+distance : 70U+distance) : 200U;
    if (State == ELEVATOR_MOVING_DOWN) return (CallDirection == DIR_DOWN) ? ((CallFloorValue < Floor) ? 10U+distance : 70U+distance) : 200U;
    if (State == ELEVATOR_DOORS_OPEN) return (Floor == CallFloorValue) ? 0U : 45U+distance;
    if (State == ELEVATOR_IDLE) return 40U+distance;
    return 900U;
}

static void DispatchHallCalls(void) {
    uint8 i, pending, commFault = IsSlaveCommFault();
    uint32 lock = Enter_Critical();
    pending = HallPending;
    Exit_Critical(lock);

    for (i = 0U; i < 6U; i++) {
        uint8 bit = (uint8)(1U << i);
        if ((pending & bit) == 0U) continue;
        uint8 floor = CallFloor(i);
        uint8 dir = CallDir(i);
        if (commFault != 0U) {
            Elevator_AddTarget(&SelfElevator, floor);
            lock = Enter_Critical(); HallPending &= (uint8)~bit; Exit_Critical(lock);
            continue;
        }
        uint16 scoreA = ScoreElevator(SelfElevator.state, SelfElevator.current_floor, SelfElevator.emergency, floor, dir);
        uint16 scoreB = ScoreElevator(SlaveStatus.state, SlaveStatus.current_floor, ((SlaveStatus.flags & IPC_FLAG_EMERGENCY) != 0U) ? 1U : 0U, floor, dir);
        if (scoreA >= 200U && scoreB >= 200U) continue;
        if (scoreA <= scoreB) Elevator_AddTarget(&SelfElevator, floor);
        else SlaveCmdMask |= FloorToMask(floor);

        lock = Enter_Critical(); HallPending &= (uint8)~bit; Exit_Critical(lock);
    }
}

void Role_Init(void) {
    Gpio_Init(PORT_INPUTS, HALL_U1_LINE, GPIO_INPUT, GPIO_PULL_UP); Exti_Init(HALL_U1_LINE, PORT_INPUTS, EXTI_EDGE_FALLING, HallU1_Callback); Exti_Enable(HALL_U1_LINE);
    Gpio_Init(PORT_INPUTS, HALL_D2_LINE, GPIO_INPUT, GPIO_PULL_UP); Exti_Init(HALL_D2_LINE, PORT_INPUTS, EXTI_EDGE_FALLING, HallD2_Callback); Exti_Enable(HALL_D2_LINE);
    Gpio_Init(PORT_INPUTS, HALL_U2_LINE, GPIO_INPUT, GPIO_PULL_UP); Exti_Init(HALL_U2_LINE, PORT_INPUTS, EXTI_EDGE_FALLING, HallU2_Callback); Exti_Enable(HALL_U2_LINE);
    Gpio_Init(PORT_INPUTS, HALL_D3_LINE, GPIO_INPUT, GPIO_PULL_UP); Exti_Init(HALL_D3_LINE, PORT_INPUTS, EXTI_EDGE_FALLING, HallD3_Callback); Exti_Enable(HALL_D3_LINE);
    Gpio_Init(PORT_INPUTS, HALL_U3_LINE, GPIO_INPUT, GPIO_PULL_UP); Exti_Init(HALL_U3_LINE, PORT_INPUTS, EXTI_EDGE_FALLING, HallU3_Callback); Exti_Enable(HALL_U3_LINE);
    Gpio_Init(PORT_INPUTS, HALL_D4_LINE, GPIO_INPUT, GPIO_PULL_UP); Exti_Init(HALL_D4_LINE, PORT_INPUTS, EXTI_EDGE_FALLING, HallD4_Callback); Exti_Enable(HALL_D4_LINE);
    
    Spi1_MasterInit();
    Usart1_TransmitString("MASTER Board A ready\r\n");
}

void Role_TimeTick(void) {
    if (SlaveAgeMs < 60000U) SlaveAgeMs++;
}

void Role_Run(void) {
    if (Flag_SpiPeriod != 0U) {
        Flag_SpiPeriod = 0U;
        if (Spi1_MasterIsBusy() != 0U) {
            if (IsSlaveCommFault() != 0U) Spi1_MasterAbort();
            return;
        }
        DispatchHallCalls();
        uint8 flag = IsSlaveCommFault() ? IPC_FLAG_COMM_FAULT : 0U;
        BuildLocalPacket(SlaveCmdMask, flag, IpcTxFrame);
        SlaveCmdMask = 0U;
        (void)Spi1_MasterStartFrame(IpcTxFrame, IpcRxFrame, SPI_FRAME_LEN, Spi_DoneCallback);
    }
    if (Flag_SpiDone != 0U) {
        Flag_SpiDone = 0U;
        IpcFrameType rx;
        if (Protocol_Parse(IpcRxFrame, &rx) == OK) {
            SlaveStatus = rx;
            SlaveAgeMs = 0U;
        }
    }
}

void Role_Telemetry(void) {
    if (Flag_Telemetry == 0U) return;
    Flag_Telemetry = 0U;
    Usart1_TransmitString("M A:F"); Usart1_TransmitUnsigned(SelfElevator.current_floor);
    Usart1_TransmitString(" S:"); Usart1_TransmitString(Elevator_StateName(SelfElevator.state));
    Usart1_TransmitString(" T:0x"); Usart1_TransmitUnsigned(SelfElevator.target_mask);
    Usart1_TransmitString(" H:0x"); Usart1_TransmitUnsigned(HallPending);
    Usart1_TransmitString(" B:F"); Usart1_TransmitUnsigned(SlaveStatus.current_floor);
    Usart1_TransmitString(" "); Usart1_TransmitString(Elevator_StateName(SlaveStatus.state));
    Usart1_TransmitString(" Comm:"); Usart1_TransmitString(IsSlaveCommFault() ? "FAULT\r\n" : "OK\r\n");
}