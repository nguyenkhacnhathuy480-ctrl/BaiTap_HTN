#include "FreeRTOS.h"
#include "board_config.h"
#include "line_sensor.h"
#include "motor_tb6612.h"
#include "platform.h"
#include "robot_tasks.h"
#include "task.h"

int main(void)
{
    Platform_Init();
    LineSensor_Init();
    Motor_Init();
    if (RobotTasks_Start() != pdPASS) {
        Motor_Stop();
        for (;;) {
        }
    }
    vTaskStartScheduler();

    Motor_Stop();
    for (;;) {
    }
}
