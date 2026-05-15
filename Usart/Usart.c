/**
 * Usart.c
 * USART1 telemetry driver. UART debug is the only polling output in this project.
 */
#include "stm32f401xe.h"
#include "Usart.h"
#include "Gpio.h"

void Usart1_Init(void) {
    Gpio_Init(GPIO_A, 9U, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_Init(GPIO_A, 10U, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_SetAF(GPIO_A, 9U, GPIO_AF7);   /* USART1_TX */
    Gpio_SetAF(GPIO_A, 10U, GPIO_AF7);  /* USART1_RX */
    Gpio_SetSpeedHigh(GPIO_A, 9U);
    Gpio_SetSpeedHigh(GPIO_A, 10U);

    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;

    /* 16 MHz / 9600 = 1666.666 => BRR = 0x0683. Same value used in tutorials. */
    USART1->BRR = 0x0683U;

    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;
    USART1->CR1 |= USART_CR1_UE;
}

uint8 Usart1_TransmitByte(uint8 Byte) {
    if ((USART1->SR & USART_SR_TXE) == 0U) {
        return NOK;
    }

    USART1->DR = Byte;
    while ((USART1->SR & USART_SR_TC) == 0U) {
        /* UART debug polling is allowed by the project statement. */
    }
    USART1->SR &= ~USART_SR_TC;
    return OK;
}

void Usart1_TransmitString(const char *Str) {
    uint32 i = 0U;
    while (Str[i] != '\0') {
        if (Usart1_TransmitByte((uint8)Str[i]) == OK) {
            i++;
        }
    }
}

void Usart1_TransmitUnsigned(uint32 Value) {
    char buf[11];
    sint32 idx = 10;
    buf[idx] = '\0';
    if (Value == 0U) {
        Usart1_TransmitByte('0');
        return;
    }
    while (Value > 0U && idx > 0) {
        idx--;
        buf[idx] = (char)('0' + (Value % 10U));
        Value /= 10U;
    }
    Usart1_TransmitString(&buf[idx]);
}
