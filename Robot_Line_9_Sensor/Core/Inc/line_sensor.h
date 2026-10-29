#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H

#include <stdint.h>

void LineSensor_Init(void);
uint16_t LineSensor_ReadMask(void);
uint8_t LineSensor_GetPosition(int32_t *position);

#endif
