#ifndef ROBOT_H
#define ROBOT_H
#include "drive.h"
typedef struct {
    uint32_t ticks;
    uint16_t raw[9];
    TrackReading track;
    DriveState state;
    WheelPWM pwm;
    int32_t base;
    uint8_t ready,buttons;
} RobotStatus;
void Robot_Start(uint8_t adc_ok);
void Robot_GetStatus(RobotStatus *out);
void Robot_RequestStop(void);
void Robot_Monitor(void *argument);
#endif
