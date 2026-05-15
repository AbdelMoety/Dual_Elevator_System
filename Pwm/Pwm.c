/**
 * Pwm.c
 * Motor LED PWM: PA6 -> TIM3_CH1 (AF2), 10 kHz at 16 MHz HSI.
 */
#include "stm32f401xe.h"
#include "Pwm.h"
#include "Gpio.h"
#include "Bit_Math.h"

void Pwm_MotorInit10kHz(void) {
    Gpio_Init(GPIO_A, 6U, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_SetAF(GPIO_A, 6U, GPIO_AF2);    /* TIM3_CH1 */
    Gpio_SetSpeedHigh(GPIO_A, 6U);

    TIM3->CR1 = 0;
    TIM3->PSC = 0U;       /* Timer clock = 16 MHz */
    TIM3->ARR = 1599U;    /* 16 MHz / (1599+1) = 10 kHz */
    TIM3->CNT = 0;

    TIM3->CCMR1 &= ~0xFFUL;
    TIM3->CCMR1 |= (6UL << 4U); /* OC1M = 110: PWM mode 1 */
    TIM3->CCMR1 |= (1UL << 3U); /* OC1PE preload */
    TIM3->CCER  |= (1UL << 0U); /* CC1E */
    TIM3->CCR1 = 0U;

    SET_BIT(TIM3->CR1, 7U);     /* ARPE */
    TIM3->EGR = TIM_EGR_UG;
    TIM3->SR = 0;
}

void Pwm_MotorSetDuty(uint8 DutyPercent) {
    if (DutyPercent > 100U) {
        DutyPercent = 100U;
    }
    TIM3->CCR1 = ((uint32)DutyPercent * TIM3->ARR) / 100UL;
}

void Pwm_MotorStart(void) {
    SET_BIT(TIM3->CCER, 0U);
    SET_BIT(TIM3->CR1, TIM_CR1_CEN_Pos);
}

void Pwm_MotorStop(void) {
    Pwm_MotorSetDuty(0U);
    CLEAR_BIT(TIM3->CR1, TIM_CR1_CEN_Pos);
}
