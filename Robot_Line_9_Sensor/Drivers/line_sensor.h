#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H
#include <stdint.h>
uint8_t Sensors_Init(void);
uint8_t Sensors_Read(uint16_t values[9]);
#endif
