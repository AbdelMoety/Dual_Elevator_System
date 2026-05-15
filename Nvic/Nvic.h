/** Nvic.h */
#ifndef NVIC_DRIVER_H
#define NVIC_DRIVER_H

#include "Std_Types.h"

void Nvic_EnableIrq(uint8 IrqNumber);
void Nvic_DisableIrq(uint8 IrqNumber);
void Nvic_SetPriority(uint8 IrqNumber, uint8 Priority);

#endif
