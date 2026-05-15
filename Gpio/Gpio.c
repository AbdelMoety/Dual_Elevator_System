/**
 * Gpio.c
 */
#include "stm32f401xe.h"
#include "Gpio.h"

static GPIO_TypeDef *Gpio_GetPort(uint8 PortName) {
    switch (PortName) {
        case GPIO_A: return GPIOA;
        case GPIO_B: return GPIOB;
        case GPIO_C: return GPIOC;
        case GPIO_D: return GPIOD;
        case GPIO_E: return GPIOE;
        case GPIO_H_PORT: return GPIOH;
        default: return GPIOA;
    }
}

void Gpio_Init(uint8 PortName, uint8 PinNumber, uint8 PinMode, uint8 DefaultState) {
    GPIO_TypeDef *gpio = Gpio_GetPort(PortName);

    gpio->MODER &= ~(0x03UL << (PinNumber * 2U));
    gpio->MODER |=  ((uint32)PinMode << (PinNumber * 2U));

    if (PinMode == GPIO_INPUT) {
        gpio->PUPDR &= ~(0x03UL << (PinNumber * 2U));
        gpio->PUPDR |=  ((uint32)DefaultState << (PinNumber * 2U));
    } else {
        gpio->OTYPER &= ~(0x01UL << PinNumber);
        gpio->OTYPER |=  ((uint32)DefaultState << PinNumber);

        /* No pull by default for outputs/alternate functions. */
        gpio->PUPDR &= ~(0x03UL << (PinNumber * 2U));
    }
}

void Gpio_SetAF(uint8 PortName, uint8 PinNumber, uint8 AF) {
    GPIO_TypeDef *gpio = Gpio_GetPort(PortName);
    uint8 regIndex = PinNumber / 8U;
    uint8 bitPos = (PinNumber % 8U) * 4U;

    gpio->AFR[regIndex] &= ~(0x0FUL << bitPos);
    gpio->AFR[regIndex] |=  ((uint32)AF << bitPos);
}

void Gpio_SetSpeedHigh(uint8 PortName, uint8 PinNumber) {
    GPIO_TypeDef *gpio = Gpio_GetPort(PortName);
    gpio->OSPEEDR &= ~(0x03UL << (PinNumber * 2U));
    gpio->OSPEEDR |=  (0x02UL << (PinNumber * 2U));
}

uint8 Gpio_WritePin(uint8 PortName, uint8 PinNumber, uint8 Data) {
    GPIO_TypeDef *gpio = Gpio_GetPort(PortName);

    if (Data == HIGH) {
        gpio->BSRR = (1UL << PinNumber);
    } else {
        gpio->BSRR = (1UL << (PinNumber + 16U));
    }
    return OK;
}

uint8 Gpio_ReadPin(uint8 PortName, uint8 PinNumber) {
    GPIO_TypeDef *gpio = Gpio_GetPort(PortName);
    return (uint8)((gpio->IDR >> PinNumber) & 1UL);
}

void Gpio_TogglePin(uint8 PortName, uint8 PinNumber) {
    GPIO_TypeDef *gpio = Gpio_GetPort(PortName);
    if ((gpio->ODR & (1UL << PinNumber)) == 0U) {
        gpio->BSRR = (1UL << PinNumber);
    } else {
        gpio->BSRR = (1UL << (PinNumber + 16U));
    }
}
