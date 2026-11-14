#include "robot_tasks.h"
#include "board_config.h"
#include "line_sensor.h"
#include "motor_tb6612.h"
#include "platform.h"
#include "task.h"

static QueueHandle_t measurement_queue;
static QueueHandle_t motor_queue;
static int32_t last_error;
static int32_t integral;

static int32_t Clamp(int32_t value, int32_t min_value, int32_t max_value)
{
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

/* Nguyễn Trung Đán: đọc và xử lý dữ liệu 9 cảm biến. */
static void SensorTask(void *argument)
{
    LineMeasurement_t measurement;
    (void)argument;
    for (;;) {
        measurement.line_seen = LineSensor_GetPosition(&measurement.position);
        (void)xQueueOverwrite(measurement_queue, &measurement);
        vTaskDelay(pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }
}

/* Nguyễn Khắc Nhật Huy: tính PID và tạo lệnh tốc độ. */
static void PIDTask(void *argument)
{
    LineMeasurement_t measurement;
    MotorCommand_t command;
    (void)argument;
    for (;;) {
        if (xQueueReceive(measurement_queue, &measurement, portMAX_DELAY) != pdPASS) continue;
        if (measurement.line_seen == 0U) {
            if (last_error < 0) {
                command.left_speed = -LINE_LOST_TURN_SPEED;
                command.right_speed = LINE_LOST_TURN_SPEED;
            } else {
                command.left_speed = LINE_LOST_TURN_SPEED;
                command.right_speed = -LINE_LOST_TURN_SPEED;
            }
        } else {
            const int32_t derivative = measurement.position - last_error;
            int32_t correction;
            integral = Clamp(integral + measurement.position, -120000, 120000);
            correction = ((LINE_PID_KP * measurement.position) +
                          (LINE_PID_KI * integral) +
                          (LINE_PID_KD * derivative)) / 1000;
            correction = Clamp(correction, -LINE_MAX_CORRECTION, LINE_MAX_CORRECTION);
            command.left_speed = Clamp(LINE_BASE_SPEED + correction,
                                        -MOTOR_PWM_MAX, MOTOR_PWM_MAX);
            command.right_speed = Clamp(LINE_BASE_SPEED - correction,
                                         -MOTOR_PWM_MAX, MOTOR_PWM_MAX);
            last_error = measurement.position;
        }
        (void)xQueueOverwrite(motor_queue, &command);
    }
}

/* Hoàng Hải Đăng: xuất chiều quay và PWM tới TB6612FNG. */
static void MotorTask(void *argument)
{
    MotorCommand_t command;
    (void)argument;
    for (;;) {
        if (xQueueReceive(motor_queue, &command, portMAX_DELAY) == pdPASS) {
            Motor_Set(command.left_speed, command.right_speed);
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

BaseType_t RobotTasks_Start(void)
{
    measurement_queue = xQueueCreate(1U, sizeof(LineMeasurement_t));
    motor_queue = xQueueCreate(1U, sizeof(MotorCommand_t));
    if ((measurement_queue == NULL) || (motor_queue == NULL)) return pdFAIL;
    if (xTaskCreate(SensorTask, "Sensor", 160U, NULL, 3U, NULL) != pdPASS) return pdFAIL;
    if (xTaskCreate(PIDTask, "PID", 192U, NULL, 4U, NULL) != pdPASS) return pdFAIL;
    if (xTaskCreate(MotorTask, "Motor", 160U, NULL, 3U, NULL) != pdPASS) return pdFAIL;
    if (xTaskCreate(StatusTask, "Status", 128U, NULL, 1U, NULL) != pdPASS) return pdFAIL;
    return pdPASS;
}

void vApplicationMallocFailedHook(void)
{
    Motor_Stop();
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    (void)task;
    (void)name;
    Motor_Stop();
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}
