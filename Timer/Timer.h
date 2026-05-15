/** Timer.h */
#ifndef TIMER_H
#define TIMER_H

#include "Std_Types.h"

#define TIMER2    2U
#define TIMER3    3U
#define TIMER4    4U
#define TIMER5    5U

typedef void (*TimerCallback)(void);

void Timer_InitPeriodicMs(uint8 TimerId, uint16 PeriodMs, TimerCallback Callback);
void Timer_Start(uint8 TimerId);
void Timer_Stop(uint8 TimerId);

#endif
