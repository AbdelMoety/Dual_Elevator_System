/** Pwm.h */
#ifndef PWM_H
#define PWM_H

#include "Std_Types.h"

void Pwm_MotorInit10kHz(void);
void Pwm_MotorSetDuty(uint8 DutyPercent);
void Pwm_MotorStart(void);
void Pwm_MotorStop(void);

#endif
