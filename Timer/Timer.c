/**
 * Timer.c
 * Hardware scheduler timer. No busy waits are used by the application FSM.
 */
#include "stm32f401xe.h"
#include "Timer.h"
#include "Nvic.h"
#include "Bit_Math.h"

static TIM_TypeDef *Timer_GetPeripheral(uint8 TimerId) {
    switch (TimerId) {
        case TIMER2: return TIM2;
        case TIMER3: return TIM3;
        case TIMER4: return TIM4;
        case TIMER5: return TIM5;
        default: return TIM2;
    }
}

static uint8 Timer_GetIrq(uint8 TimerId) {
    switch (TimerId) {
        case TIMER2: return 28U;
        case TIMER3: return 29U;
        case TIMER4: return 30U;
        case TIMER5: return 50U;
        default: return 28U;
    }
}

static TimerCallback Timer2_Callback = 0;
static TimerCallback Timer3_Callback = 0;
static TimerCallback Timer4_Callback = 0;
static TimerCallback Timer5_Callback = 0;

static TimerCallback *Timer_GetCallbackSlot(uint8 TimerId) {
    switch (TimerId) {
        case TIMER2: return &Timer2_Callback;
        case TIMER3: return &Timer3_Callback;
        case TIMER4: return &Timer4_Callback;
        case TIMER5: return &Timer5_Callback;
        default: return &Timer2_Callback;
    }
}

void Timer_InitPeriodicMs(uint8 TimerId, uint16 PeriodMs, TimerCallback Callback) {
    TIM_TypeDef *timer = Timer_GetPeripheral(TimerId);
    TimerCallback *slot = Timer_GetCallbackSlot(TimerId);

    *slot = Callback;

    timer->CR1 = 0;
    timer->PSC = 15999U;                 /* 16 MHz / 16000 = 1 kHz */
    timer->ARR = (uint16)(PeriodMs - 1U); /* Period in milliseconds */
    timer->CNT = 0;
    timer->EGR = TIM_EGR_UG;
    timer->SR = 0;
    timer->DIER = TIM_DIER_UIE;

    Nvic_SetPriority(Timer_GetIrq(TimerId), 6U);
    Nvic_EnableIrq(Timer_GetIrq(TimerId));
}

void Timer_Start(uint8 TimerId) {
    SET_BIT(Timer_GetPeripheral(TimerId)->CR1, TIM_CR1_CEN_Pos);
}

void Timer_Stop(uint8 TimerId) {
    CLEAR_BIT(Timer_GetPeripheral(TimerId)->CR1, TIM_CR1_CEN_Pos);
}

static void Timer_HandleIrq(TIM_TypeDef *timer, TimerCallback cb) {
    if ((timer->SR & TIM_SR_UIF) != 0U) {
        timer->SR &= ~TIM_SR_UIF;
        if (cb != 0) {
            cb();
        }
    }
}

void TIM2_IRQHandler(void) { Timer_HandleIrq(TIM2, Timer2_Callback); }
void TIM3_IRQHandler(void) { Timer_HandleIrq(TIM3, Timer3_Callback); }
void TIM4_IRQHandler(void) { Timer_HandleIrq(TIM4, Timer4_Callback); }
void TIM5_IRQHandler(void) { Timer_HandleIrq(TIM5, Timer5_Callback); }
