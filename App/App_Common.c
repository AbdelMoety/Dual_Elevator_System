/** App_Common.c */
#include "App_Internal.h"
#include "App.h"
#include "Rcc.h"
#include "Gpio.h"
#include "Exti.h"
#include "Nvic.h"
#include "Pwm.h"
#include "Usart.h"
#include "Spi.h"
#include "Critical.h"

#define PORT_INPUTS                 GPIO_D
#define CABIN_F1_LINE               0U
#define CABIN_F2_LINE               1U
#define CABIN_F3_LINE               2U
#define CABIN_F4_LINE               3U
#define SENSOR_F1_LINE              11U
#define SENSOR_F2_LINE              12U
#define SENSOR_F3_LINE              13U
#define SENSOR_F4_LINE              14U
#define EMERGENCY_LINE              15U

ElevatorType SelfElevator;
volatile uint8 Flag_SpiPeriod = 0U;
volatile uint8 Flag_SpiDone = 0U;
volatile uint8 Flag_Telemetry = 0U;
uint8 IpcTxFrame[SPI_FRAME_LEN];
uint8 IpcRxFrame[SPI_FRAME_LEN];

static volatile uint8 CabinPendingMask = 0U;
static volatile uint8 SensorPendingMask = 0U;
static volatile uint8 EmergencyPending = 0U;
static volatile uint8 Flag_FsmStep = 0U;
static volatile uint32 TickMs = 0U;
static uint8 LocalSeq = 0U;

uint8 FloorToMask(uint8 Floor) {
    return (uint8)(1U << (Floor - 1U));
}

void Spi_DoneCallback(void) {
    Flag_SpiDone = 1U;
}

static void Cabin1_Callback(void) { CabinPendingMask |= FloorToMask(1U); }
static void Cabin2_Callback(void) { CabinPendingMask |= FloorToMask(2U); }
static void Cabin3_Callback(void) { CabinPendingMask |= FloorToMask(3U); }
static void Cabin4_Callback(void) { CabinPendingMask |= FloorToMask(4U); }
static void Sensor1_Callback(void) { SensorPendingMask |= FloorToMask(1U); }
static void Sensor2_Callback(void) { SensorPendingMask |= FloorToMask(2U); }
static void Sensor3_Callback(void) { SensorPendingMask |= FloorToMask(3U); }
static void Sensor4_Callback(void) { SensorPendingMask |= FloorToMask(4U); }
static void Emergency_Callback(void) { EmergencyPending = 1U; }

void BuildLocalPacket(uint8 CommandMask, uint8 CommFlag, uint8 OutFrame[SPI_FRAME_LEN]) {
    IpcFrameType pkt;
    pkt.seq = LocalSeq++;
    pkt.state = SelfElevator.state;
    pkt.current_floor = SelfElevator.current_floor;
    pkt.target_mask = SelfElevator.target_mask;
    pkt.flags = Elevator_GetFlags(&SelfElevator);
    if (SelfElevator.emergency != 0U) pkt.flags |= IPC_FLAG_EMERGENCY;
    pkt.flags |= CommFlag; /* Master passes COMM_FAULT, Slave passes INDEPENDENT */
    pkt.cmd_mask = CommandMask;
    Protocol_Build(&pkt, OutFrame);
}

static void Configure_OneInput(uint8 Line, ExtiCallback Cb) {
    Gpio_Init(PORT_INPUTS, Line, GPIO_INPUT, GPIO_PULL_UP);
    Exti_Init(Line, PORT_INPUTS, EXTI_EDGE_FALLING, Cb);
    Exti_Enable(Line);
}

static void ConsumeInputEvents(void) {
    uint8 cabin, sensors, emergency, floor;
    uint32 lock = Enter_Critical();
    cabin = CabinPendingMask;
    sensors = SensorPendingMask;
    emergency = EmergencyPending;
    CabinPendingMask = 0U;
    SensorPendingMask = 0U;
    EmergencyPending = 0U;
    Exit_Critical(lock);

    if (emergency != 0U) Elevator_ToggleEmergency(&SelfElevator);

    for (floor = FLOOR_MIN; floor <= FLOOR_MAX; floor++) {
        if ((cabin & FloorToMask(floor)) != 0U) Elevator_AddTarget(&SelfElevator, floor);
        if ((sensors & FloorToMask(floor)) != 0U) Elevator_SetFloorFromSensor(&SelfElevator, floor);
    }
}

static void Scheduler_1msCallback(void) {
    static uint8 fsmCounter = 0U;   
    static uint8 spiCounter = 0U;
    static uint16 telemetryCounter = 0U;

    TickMs++;
    Role_TimeTick(); /* Tell the Master/Slave that 1ms passed */

    if (++fsmCounter >= FSM_STEP_MS) { fsmCounter = 0U; Flag_FsmStep = 1U; }
    if (++spiCounter >= SPI_PERIOD_MS) { spiCounter = 0U; Flag_SpiPeriod = 1U; }
    if (++telemetryCounter >= TELEMETRY_PERIOD_MS) { telemetryCounter = 0U; Flag_Telemetry = 1U; }
}

void App_Init(void) {
    Rcc_Init();
    Rcc_Enable(RCC_GPIOA); Rcc_Enable(RCC_GPIOB); Rcc_Enable(RCC_GPIOD);
    Rcc_Enable(RCC_SYSCFG); Rcc_Enable(RCC_TIM3); Rcc_Enable(RCC_USART1); Rcc_Enable(RCC_SPI2);

    Pwm_MotorInit10kHz(); Pwm_MotorStart(); Pwm_MotorSetDuty(0U);
    Usart1_Init();
    Elevator_Init(&SelfElevator, 1U);

    /* Inputs Init */
    Nvic_SetPriority(6U, 4U); Nvic_SetPriority(7U, 4U); Nvic_SetPriority(8U, 4U);
    Nvic_SetPriority(9U, 4U); Nvic_SetPriority(10U, 4U); Nvic_SetPriority(23U, 4U);
    Nvic_SetPriority(40U, 0U); 

    Configure_OneInput(CABIN_F1_LINE, Cabin1_Callback); Configure_OneInput(CABIN_F2_LINE, Cabin2_Callback);
    Configure_OneInput(CABIN_F3_LINE, Cabin3_Callback); Configure_OneInput(CABIN_F4_LINE, Cabin4_Callback);
    Configure_OneInput(SENSOR_F1_LINE, Sensor1_Callback); Configure_OneInput(SENSOR_F2_LINE, Sensor2_Callback);
    Configure_OneInput(SENSOR_F3_LINE, Sensor3_Callback); Configure_OneInput(SENSOR_F4_LINE, Sensor4_Callback);
    Configure_OneInput(EMERGENCY_LINE, Emergency_Callback);

    Role_Init(); /* Initialize Master or Slave specific hardware */

    SysTick->LOAD = 16000U - 1U; 
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void App_Run(void) {
    while (1) {
        ConsumeInputEvents();
        if (Flag_FsmStep != 0U) {
            Flag_FsmStep = 0U;
            Elevator_FsmStep(&SelfElevator, FSM_STEP_MS);
        }
        Role_Run();       /* Run Master or Slave specific logic */
        Role_Telemetry(); /* Run Master or Slave telemetry */
    }
}

void SysTick_Handler(void) { Scheduler_1msCallback(); }