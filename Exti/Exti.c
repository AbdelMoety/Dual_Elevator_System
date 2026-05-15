/**
 * Exti.c
 */
#include "stm32f401xe.h"
#include "Exti.h"
#include "Nvic.h"
#include "Gpio.h"

static ExtiCallback ExtiCallbacks[16] = {0};
static uint8 ExtiLineToIrq[16] = {6, 7, 8, 9, 10, 23, 23, 23, 23, 23, 40, 40, 40, 40, 40, 40};

static uint8 Exti_PortCode(uint8 PortName) {
    switch (PortName) {
        case GPIO_A: return 0U;
        case GPIO_B: return 1U;
        case GPIO_C: return 2U;
        case GPIO_D: return 3U;
        case GPIO_E: return 4U;
        case GPIO_H_PORT: return 7U;
        default: return 0U;
    }
}

void Exti_Init(uint8 LineNumber, uint8 PortName, uint8 EdgeType, ExtiCallback Callback) {
    uint8 index = LineNumber / 4U;
    uint8 shift = (LineNumber % 4U) * 4U;

    SYSCFG->EXTICR[index] &= ~(0x0FUL << shift);
    SYSCFG->EXTICR[index] |= ((uint32)Exti_PortCode(PortName) << shift);

    ExtiCallbacks[LineNumber] = Callback;

    EXTI->RTSR &= ~(1UL << LineNumber);
    EXTI->FTSR &= ~(1UL << LineNumber);

    if (EdgeType == EXTI_EDGE_RISING || EdgeType == EXTI_EDGE_BOTH) {
        EXTI->RTSR |= (1UL << LineNumber);
    }
    if (EdgeType == EXTI_EDGE_FALLING || EdgeType == EXTI_EDGE_BOTH) {
        EXTI->FTSR |= (1UL << LineNumber);
    }

    Exti_ClearPending(LineNumber);
}

void Exti_Enable(uint8 LineNumber) {
    EXTI->IMR |= (1UL << LineNumber);
    Nvic_EnableIrq(ExtiLineToIrq[LineNumber]);
}

void Exti_Disable(uint8 LineNumber) {
    EXTI->IMR &= ~(1UL << LineNumber);
    Nvic_DisableIrq(ExtiLineToIrq[LineNumber]);
}

void Exti_ClearPending(uint8 LineNumber) {
    EXTI->PR = (1UL << LineNumber);
}

static void Exti_RunLine(uint8 LineNumber) {
    if ((EXTI->PR & (1UL << LineNumber)) != 0U) {
        if (ExtiCallbacks[LineNumber] != 0) {
            ExtiCallbacks[LineNumber]();
        }
        Exti_ClearPending(LineNumber);
    }
}

void EXTI0_IRQHandler(void) { Exti_RunLine(0); }
void EXTI1_IRQHandler(void) { Exti_RunLine(1); }
void EXTI2_IRQHandler(void) { Exti_RunLine(2); }
void EXTI3_IRQHandler(void) { Exti_RunLine(3); }
void EXTI4_IRQHandler(void) { Exti_RunLine(4); }

void EXTI9_5_IRQHandler(void) {
    Exti_RunLine(5);
    Exti_RunLine(6);
    Exti_RunLine(7);
    Exti_RunLine(8);
    Exti_RunLine(9);
}

void EXTI15_10_IRQHandler(void) {
    /* Line 15 is the emergency stop line, so service it first inside the shared IRQ. */
    Exti_RunLine(15);
    Exti_RunLine(10);
    Exti_RunLine(11);
    Exti_RunLine(12);
    Exti_RunLine(13);
    Exti_RunLine(14);
}
