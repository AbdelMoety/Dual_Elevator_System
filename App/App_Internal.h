/** App_Internal.h */
#ifndef APP_INTERNAL_H
#define APP_INTERNAL_H

#include "stm32f401xe.h"
#include "Elevator.h"
#include "Protocol.h"

#define FSM_STEP_MS                 10U
#define SPI_PERIOD_MS               25U
#define TELEMETRY_PERIOD_MS         200U
#define COMM_TIMEOUT_MS             200U

/* Shared State Variables */
extern ElevatorType SelfElevator;
extern volatile uint8 Flag_SpiPeriod;
extern volatile uint8 Flag_SpiDone;
extern volatile uint8 Flag_Telemetry;
extern uint8 IpcTxFrame[SPI_FRAME_LEN];
extern uint8 IpcRxFrame[SPI_FRAME_LEN];

/* Shared Helper Functions */
uint8 FloorToMask(uint8 Floor);
void BuildLocalPacket(uint8 CommandMask, uint8 CommFlag, uint8 OutFrame[SPI_FRAME_LEN]);
void Spi_DoneCallback(void);

/* Role Interface (Implemented in Master/Slave files) */
void Role_Init(void);
void Role_Run(void);
void Role_Telemetry(void);
void Role_TimeTick(void);

#endif