#ifndef ROBOT_TASKS_H
#define ROBOT_TASKS_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

typedef struct {
    int32_t position;
    uint8_t line_seen;
} LineMeasurement_t;

typedef struct {
    int32_t left_speed;
    int32_t right_speed;
} MotorCommand_t;

BaseType_t RobotTasks_Start(void);

#endif
