/**
 * Rcc.h
 */
#ifndef RCC_H
#define RCC_H

#include "Std_Types.h"

#define RCC_AHB1   0U
#define RCC_AHB2   1U
#define RCC_APB1   2U
#define RCC_APB2   3U

#define RCC_GPIOA   ((RCC_AHB1 * 32U) + 0U)
#define RCC_GPIOB   ((RCC_AHB1 * 32U) + 1U)
#define RCC_GPIOC   ((RCC_AHB1 * 32U) + 2U)
#define RCC_GPIOD   ((RCC_AHB1 * 32U) + 3U)
#define RCC_GPIOE   ((RCC_AHB1 * 32U) + 4U)
#define RCC_GPIOH   ((RCC_AHB1 * 32U) + 7U)

#define RCC_TIM2    ((RCC_APB1 * 32U) + 0U)
#define RCC_TIM3    ((RCC_APB1 * 32U) + 1U)
#define RCC_TIM4    ((RCC_APB1 * 32U) + 2U)
#define RCC_TIM5    ((RCC_APB1 * 32U) + 3U)

#define RCC_USART1  ((RCC_APB2 * 32U) + 4U)
#define RCC_SPI2    ((RCC_APB1 * 32U) + 14U)
#define RCC_SPI1    ((RCC_APB2 * 32U) + 12U)
#define RCC_SYSCFG  ((RCC_APB2 * 32U) + 14U)

void Rcc_Init(void);
void Rcc_Enable(uint8 PeripheralId);
void Rcc_Disable(uint8 PeripheralId);
uint32 Rcc_GetSystemClockHz(void);

#endif
