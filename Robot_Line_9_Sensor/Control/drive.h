#ifndef DRIVE_H
#define DRIVE_H
#include "tracker.h"
typedef enum { DRIVE_IDLE, DRIVE_ACTIVE, DRIVE_NO_LINE, DRIVE_WIDE, DRIVE_AMBIGUOUS, DRIVE_TIMING, DRIVE_ADC, DRIVE_CAL, DRIVE_CORNER_LEFT, DRIVE_CORNER_RIGHT, DRIVE_CORNER_TIMEOUT, DRIVE_DROPOUT } DriveState;
typedef struct { int32_t left,right; } WheelPWM;
typedef struct {
    DriveState state;
    uint8_t good,have_history,edge_frames,center_frames;
    int8_t edge_side;
    uint32_t last,corner_since;
    int32_t base,prior,d_q8,integral,throttle;
    WheelPWM last_pwm;
    uint32_t dropout_since;
    uint8_t dropout_good;
} Drive;
void Drive_Init(Drive *d);
void Drive_SetSpeed(Drive *d,int32_t base);
WheelPWM Drive_Update(Drive *d,TrackReading line,uint32_t now,uint8_t toggle,uint8_t stop);
#endif
