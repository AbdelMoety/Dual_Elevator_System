/**
 * Elevator.c
 * Minimal finite-state machine for one elevator car.
 */
#include "Elevator.h"
#include "Pwm.h"

#define DOOR_OPEN_MS       600U
#define FLOOR_TRAVEL_MS    400U

static uint8 FloorToMask(uint8 Floor) {
    return (uint8)(1U << (Floor - 1U));
}

static uint8 HasTarget(const ElevatorType *Elevator, uint8 Floor) {
    return (Elevator->target_mask & FloorToMask(Floor)) != 0U;
}

static void ClearTarget(ElevatorType *Elevator, uint8 Floor) {
    Elevator->target_mask &= (uint8)~FloorToMask(Floor);
}

static uint8 PickNearestTarget(const ElevatorType *Elevator) {
    uint8 floor;
    uint8 best = 0U;
    uint8 bestDist = 255U;

    for (floor = FLOOR_MIN; floor <= FLOOR_MAX; floor++) {
        if (HasTarget(Elevator, floor)) {
            uint8 dist = (floor > Elevator->current_floor) ?
                         (uint8)(floor - Elevator->current_floor) :
                         (uint8)(Elevator->current_floor - floor);
            if (dist < bestDist) {
                bestDist = dist;
                best = floor;
            }
        }
    }
    return best;
}

static uint8 FindTargetAbove(const ElevatorType *Elevator) {
    uint8 floor;
    for (floor = (uint8)(Elevator->current_floor + 1U); floor <= FLOOR_MAX; floor++) {
        if (HasTarget(Elevator, floor)) {
            return floor;
        }
    }
    return 0U;
}

static uint8 FindTargetBelow(const ElevatorType *Elevator) {
    sint8 floor;
    for (floor = (sint8)(Elevator->current_floor - 1U); floor >= (sint8)FLOOR_MIN; floor--) {
        if (HasTarget(Elevator, (uint8)floor)) {
            return (uint8)floor;
        }
    }
    return 0U;
}

static void OpenDoor(ElevatorType *Elevator) {
    Elevator->state = ELEVATOR_DOORS_OPEN;
    Elevator->door_timer_ms = DOOR_OPEN_MS;
    Elevator->travel_timer_ms = 0U;
    Pwm_MotorSetDuty(0U);
}

void Elevator_Init(ElevatorType *Elevator, uint8 StartFloor) {
    Elevator->current_floor = StartFloor;
    Elevator->state = ELEVATOR_IDLE;
    Elevator->target_mask = 0U;
    Elevator->emergency = 0U;
    Elevator->door_timer_ms = 0U;
    Elevator->travel_timer_ms = 0U;
}

void Elevator_AddTarget(ElevatorType *Elevator, uint8 Floor) {
    if (Floor >= FLOOR_MIN && Floor <= FLOOR_MAX) {
        Elevator->target_mask |= FloorToMask(Floor);
    }
}

void Elevator_SetFloorFromSensor(ElevatorType *Elevator, uint8 Floor) {
    if (Floor >= FLOOR_MIN && Floor <= FLOOR_MAX) {
        Elevator->current_floor = Floor;
        if (HasTarget(Elevator, Floor)) {
            ClearTarget(Elevator, Floor);
            OpenDoor(Elevator);
        }
    }
}

void Elevator_ToggleEmergency(ElevatorType *Elevator) {
    if (Elevator->emergency == 0U) {
        Elevator->emergency = 1U;
        Elevator->state = ELEVATOR_EMERGENCY;
        Pwm_MotorSetDuty(0U);
    } else {
        Elevator->emergency = 0U;
        Elevator->state = ELEVATOR_IDLE;
    }
}

void Elevator_FsmStep(ElevatorType *Elevator, uint16 DeltaMs) {
    uint8 target;

    if (Elevator->emergency != 0U) {
        Elevator->state = ELEVATOR_EMERGENCY;
        Pwm_MotorSetDuty(0U);
        return;
    }

    if (Elevator->state == ELEVATOR_DOORS_OPEN) {
        if (Elevator->door_timer_ms > DeltaMs) {
            Elevator->door_timer_ms -= DeltaMs;
        } else {
            Elevator->door_timer_ms = 0U;
            Elevator->state = ELEVATOR_IDLE;
        }
        Pwm_MotorSetDuty(0U);
        return;
    }

    if (HasTarget(Elevator, Elevator->current_floor)) {
        ClearTarget(Elevator, Elevator->current_floor);
        OpenDoor(Elevator);
        return;
    }

    if (Elevator->state == ELEVATOR_IDLE) {
        target = PickNearestTarget(Elevator);
        if (target == 0U) {
            Pwm_MotorSetDuty(0U);
            return;
        }
        Elevator->state = (target > Elevator->current_floor) ? ELEVATOR_MOVING_UP : ELEVATOR_MOVING_DOWN;
        Elevator->travel_timer_ms = 0U;
    }

    if (Elevator->state == ELEVATOR_MOVING_UP) {
        target = FindTargetAbove(Elevator);
        if (target == 0U) {
            Elevator->state = ELEVATOR_IDLE;
            Pwm_MotorSetDuty(0U);
            return;
        }
        Pwm_MotorSetDuty(((uint8)(target - Elevator->current_floor) <= 1U) ? 20U : 100U);
        Elevator->travel_timer_ms += DeltaMs;
        if (Elevator->travel_timer_ms >= FLOOR_TRAVEL_MS) {
            Elevator->travel_timer_ms = 0U;
            if (Elevator->current_floor < FLOOR_MAX) {
                Elevator->current_floor++;
            }
            if (HasTarget(Elevator, Elevator->current_floor)) {
                ClearTarget(Elevator, Elevator->current_floor);
                OpenDoor(Elevator);
            }
        }
    } else if (Elevator->state == ELEVATOR_MOVING_DOWN) {
        target = FindTargetBelow(Elevator);
        if (target == 0U) {
            Elevator->state = ELEVATOR_IDLE;
            Pwm_MotorSetDuty(0U);
            return;
        }

        Pwm_MotorSetDuty(((uint8)(Elevator->current_floor - target) <= 1U) ? 20U : 100U);
        Elevator->travel_timer_ms += DeltaMs;
        if (Elevator->travel_timer_ms >= FLOOR_TRAVEL_MS) {
            Elevator->travel_timer_ms = 0U;
            if (Elevator->current_floor > FLOOR_MIN) {
                Elevator->current_floor--;
            }
            
            if (HasTarget(Elevator, Elevator->current_floor)) {
                ClearTarget(Elevator, Elevator->current_floor);
                OpenDoor(Elevator);
            }
        }
    }
}

uint8 Elevator_GetDirection(const ElevatorType *Elevator) {
    if (Elevator->state == ELEVATOR_MOVING_UP) {
        return DIR_UP;
    }
    if (Elevator->state == ELEVATOR_MOVING_DOWN) {
        return DIR_DOWN;
    }
    return DIR_NONE;
}

uint8 Elevator_GetFlags(const ElevatorType *Elevator) {
    uint8 flags = 0U;
    uint8 dir = Elevator_GetDirection(Elevator);
    if (Elevator->emergency != 0U) {
        flags |= (1U << 0U);
    }
    if (dir == DIR_UP) {
        flags |= (1U << 2U);
    } else if (dir == DIR_DOWN) {
        flags |= (1U << 3U);
    }
    return flags;
}

const char *Elevator_StateName(uint8 State) {
    switch (State) {
        case ELEVATOR_IDLE: return "IDLE";
        case ELEVATOR_MOVING_UP: return "UP";
        case ELEVATOR_MOVING_DOWN: return "DOWN";
        case ELEVATOR_DOORS_OPEN: return "DOOR";
        case ELEVATOR_EMERGENCY: return "EMERG";
        default: return "?";
    }
}
