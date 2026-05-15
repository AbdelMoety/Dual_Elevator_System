/**
 * Nvic.c
 * Small NVIC driver in the same register-oriented style as the tutorials.
 */
#include "Nvic.h"

#define NVIC_ISER_BASE   ((volatile uint32 *)0xE000E100UL)
#define NVIC_ICER_BASE   ((volatile uint32 *)0xE000E180UL)
#define NVIC_IPR_BASE    ((volatile uint8  *)0xE000E400UL)

void Nvic_EnableIrq(uint8 IrqNumber) {
    NVIC_ISER_BASE[IrqNumber / 32U] = (1UL << (IrqNumber % 32U));
}

void Nvic_DisableIrq(uint8 IrqNumber) {
    NVIC_ICER_BASE[IrqNumber / 32U] = (1UL << (IrqNumber % 32U));
}

void Nvic_SetPriority(uint8 IrqNumber, uint8 Priority) {
    /* STM32F401 implements 4 priority bits, stored in bits[7:4].
       Smaller numerical value means higher priority. */
    NVIC_IPR_BASE[IrqNumber] = (uint8)(Priority << 4U);
}
