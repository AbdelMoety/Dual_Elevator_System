/**
 * Rcc.c
 * Minimal clock driver: keep the default 16 MHz HSI clock used in the tutorials.
 */
#include "stm32f401xe.h"
#include "Rcc.h"
#include "Bit_Math.h"

void Rcc_Init(void) {
    /* HSI ON. After reset it is already selected as SYSCLK, but this keeps the
       startup explicit for Proteus and for reading the code. */
    SET_BIT(RCC->CR, RCC_CR_HSION_Pos);
    while (!READ_BIT(RCC->CR, RCC_CR_HSIRDY_Pos)) {
        /* wait until HSI is ready */
    }

    /* SYSCLK = HSI, AHB/APB prescalers = 1 */
    RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
}

void Rcc_Enable(uint8 PeripheralId) {
    uint8 BusId = PeripheralId / 32U;
    uint8 BitPosition = PeripheralId % 32U;

    switch (BusId) {
        case RCC_AHB1: SET_BIT(RCC->AHB1ENR, BitPosition); break;
        case RCC_AHB2: SET_BIT(RCC->AHB2ENR, BitPosition); break;
        case RCC_APB1: SET_BIT(RCC->APB1ENR, BitPosition); break;
        case RCC_APB2: SET_BIT(RCC->APB2ENR, BitPosition); break;
        default: break;
    }

    /* Small read-back delay after enabling a peripheral clock. */
    (void)RCC->AHB1ENR;
}

void Rcc_Disable(uint8 PeripheralId) {
    uint8 BusId = PeripheralId / 32U;
    uint8 BitPosition = PeripheralId % 32U;

    switch (BusId) {
        case RCC_AHB1: CLEAR_BIT(RCC->AHB1ENR, BitPosition); break;
        case RCC_AHB2: CLEAR_BIT(RCC->AHB2ENR, BitPosition); break;
        case RCC_APB1: CLEAR_BIT(RCC->APB1ENR, BitPosition); break;
        case RCC_APB2: CLEAR_BIT(RCC->APB2ENR, BitPosition); break;
        default: break;
    }
}

uint32 Rcc_GetSystemClockHz(void) {
    return 16000000UL;
}
