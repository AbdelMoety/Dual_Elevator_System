/** Exti.h */
#ifndef EXTI_DRIVER_H
#define EXTI_DRIVER_H

#include "Std_Types.h"

#define EXTI_EDGE_RISING   0U
#define EXTI_EDGE_FALLING  1U
#define EXTI_EDGE_BOTH     2U

typedef void (*ExtiCallback)(void);

void Exti_Init(uint8 LineNumber, uint8 PortName, uint8 EdgeType, ExtiCallback Callback);
void Exti_Enable(uint8 LineNumber);
void Exti_Disable(uint8 LineNumber);
void Exti_ClearPending(uint8 LineNumber);

#endif
