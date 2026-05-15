/**
 * Critical.c
 * Protects shared ISR/main data without hiding the Cortex-M mechanism.
 */
#include "stm32f401xe.h"
#include "Critical.h"

uint32 Enter_Critical(void) {
    uint32 primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

void Exit_Critical(uint32 PreviousPrimask) {
    if ((PreviousPrimask & 1UL) == 0UL) {
        __enable_irq();
    }
}
