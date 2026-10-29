#include "FreeRTOS.h"
#include "board_config.h"
#include "line_sensor.h"
#include "motor_tb6612.h"
#include "platform.h"
#include "queue.h"
#include "task.h"

typedef struct {
    int32_t position;
    uint8_t line_seen;
} LineMeasurement_t;

static QueueHandle_t line_queue;
static int32_t last_error;
static int32_t integral;

static int32_t Clamp(int32_t value, int32_t min_value, int32_t max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static void SensorTask(void *argument)
{
    LineMeasurement_t measurement;
    (void)argument;

    for (;;) {
        measurement.line_seen = LineSensor_GetPosition(&measurement.position);
        (void)xQueueOverwrite(line_queue, &measurement);
        vTaskDelay(pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }
}

static void ControlTask(void *argument)
{
    LineMeasurement_t measurement;
    (void)argument;

    for (;;) {
        if (xQueueReceive(line_queue, &measurement, portMAX_DELAY) == pdPASS) {
            int32_t correction;
            int32_t derivative;
            int32_t left_speed;
            int32_t right_speed;

            if (measurement.line_seen == 0U) {
                if (last_error < 0) {
                    Motor_Set(-LINE_LOST_TURN_SPEED, LINE_LOST_TURN_SPEED);
                } else {
                    Motor_Set(LINE_LOST_TURN_SPEED, -LINE_LOST_TURN_SPEED);
                }
                continue;
            }

            integral = Clamp(integral + measurement.position, -120000, 120000);
            derivative = measurement.position - last_error;
            correction = ((LINE_PID_KP * measurement.position) +
                          (LINE_PID_KI * integral) +
                          (LINE_PID_KD * derivative)) / 1000;
            correction = Clamp(correction, -LINE_MAX_CORRECTION,
                               LINE_MAX_CORRECTION);

            left_speed = Clamp(LINE_BASE_SPEED + correction,
                               -MOTOR_PWM_MAX, MOTOR_PWM_MAX);
            right_speed = Clamp(LINE_BASE_SPEED - correction,
                                -MOTOR_PWM_MAX, MOTOR_PWM_MAX);
            Motor_Set(left_speed, right_speed);
            last_error = measurement.position;
        }
    }
}

static void StatusTask(void *argument)
{
    (void)argument;
    for (;;) {
        Platform_StatusLedToggle();
        vTaskDelay(pdMS_TO_TICKS(250U));
    }
}

void vApplicationMallocFailedHook(void)
{
    Motor_Stop();
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    (void)task;
    (void)name;
    Motor_Stop();
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

int main(void)
{
    Platform_Init();
    LineSensor_Init();
    Motor_Init();
    line_queue = xQueueCreate(1U, sizeof(LineMeasurement_t));

    if (line_queue == NULL) {
        Motor_Stop();
        for (;;) {
        }
    }

    (void)xTaskCreate(SensorTask, "Sensor", 160U, NULL, 3U, NULL);
    (void)xTaskCreate(ControlTask, "Control", 192U, NULL, 4U, NULL);
    (void)xTaskCreate(StatusTask, "Status", 128U, NULL, 1U, NULL);
    vTaskStartScheduler();

    Motor_Stop();
    for (;;) {
    }
}
