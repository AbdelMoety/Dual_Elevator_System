/**
 * Gpio.h
 */
#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "Std_Types.h"

/* Port names */
#define GPIO_A     'A'
#define GPIO_B     'B'
#define GPIO_C     'C'
#define GPIO_D     'D'
#define GPIO_E     'E'
#define GPIO_H_PORT  'H'

/* Pin modes */
#define GPIO_INPUT      0x00U
#define GPIO_OUTPUT     0x01U
#define GPIO_AF         0x02U
#define GPIO_ANALOG     0x03U

/* Output type when mode is output/AF */
#define GPIO_PUSH_PULL  0x00U
#define GPIO_OPEN_DRAIN 0x01U

/* Pull state when mode is input */
#define GPIO_NO_PULL_DOWN 0x00U
#define GPIO_PULL_UP      0x01U
#define GPIO_PULL_DOWN    0x02U

#define LOW     0U
#define HIGH    1U

#define GPIO_AF0   0x00U
#define GPIO_AF1   0x01U
#define GPIO_AF2   0x02U
#define GPIO_AF3   0x03U
#define GPIO_AF4   0x04U
#define GPIO_AF5   0x05U
#define GPIO_AF6   0x06U
#define GPIO_AF7   0x07U
#define GPIO_AF8   0x08U
#define GPIO_AF9   0x09U
#define GPIO_AF10  0x0AU
#define GPIO_AF11  0x0BU
#define GPIO_AF12  0x0CU
#define GPIO_AF13  0x0DU
#define GPIO_AF14  0x0EU
#define GPIO_AF15  0x0FU

void Gpio_Init(uint8 PortName, uint8 PinNumber, uint8 PinMode, uint8 DefaultState);
void Gpio_SetAF(uint8 PortName, uint8 PinNumber, uint8 AF);
void Gpio_SetSpeedHigh(uint8 PortName, uint8 PinNumber);
uint8 Gpio_WritePin(uint8 PortName, uint8 PinNumber, uint8 Data);
uint8 Gpio_ReadPin(uint8 PortName, uint8 PinNumber);
void Gpio_TogglePin(uint8 PortName, uint8 PinNumber);

#endif
