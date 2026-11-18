#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H
#include <stdint.h>
void Motors_Init(void);
void Motors_Set(int32_t left, int32_t right);
void Motors_Stop(void);
#endif
