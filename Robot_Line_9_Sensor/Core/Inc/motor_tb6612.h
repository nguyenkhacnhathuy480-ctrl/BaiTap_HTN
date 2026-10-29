#ifndef MOTOR_TB6612_H
#define MOTOR_TB6612_H

#include <stdint.h>

void Motor_Init(void);
void Motor_Set(int32_t left_speed, int32_t right_speed);
void Motor_Stop(void);

#endif
