/** Critical.h */
#ifndef CRITICAL_H
#define CRITICAL_H

#include "Std_Types.h"

uint32 Enter_Critical(void);
void Exit_Critical(uint32 PreviousPrimask);

#endif
