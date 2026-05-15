/** Usart.h */
#ifndef USART_DRIVER_H
#define USART_DRIVER_H

#include "Std_Types.h"

void Usart1_Init(void);
uint8 Usart1_TransmitByte(uint8 Byte);
void Usart1_TransmitString(const char *Str);
void Usart1_TransmitUnsigned(uint32 Value);

#endif
