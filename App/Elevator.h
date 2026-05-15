/** Elevator.h */
#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "Std_Types.h"

#define FLOOR_MIN       1U
#define FLOOR_MAX       4U
#define DIR_NONE        0U
#define DIR_UP          1U
#define DIR_DOWN        2U

typedef enum {
    ELEVATOR_IDLE = 0,
    ELEVATOR_MOVING_UP,
    ELEVATOR_MOVING_DOWN,
    ELEVATOR_DOORS_OPEN,
    ELEVATOR_EMERGENCY
} ElevatorStateType;

typedef struct {
    volatile uint8 current_floor;
    volatile uint8 state;
    volatile uint8 target_mask;
    volatile uint8 emergency;
    uint16 door_timer_ms;
    uint16 travel_timer_ms;
} ElevatorType;

void Elevator_Init(ElevatorType *Elevator, uint8 StartFloor);
void Elevator_AddTarget(ElevatorType *Elevator, uint8 Floor);
void Elevator_SetFloorFromSensor(ElevatorType *Elevator, uint8 Floor);
void Elevator_ToggleEmergency(ElevatorType *Elevator);
void Elevator_FsmStep(ElevatorType *Elevator, uint16 DeltaMs);
uint8 Elevator_GetDirection(const ElevatorType *Elevator);
uint8 Elevator_GetFlags(const ElevatorType *Elevator);
const char *Elevator_StateName(uint8 State);

#endif
