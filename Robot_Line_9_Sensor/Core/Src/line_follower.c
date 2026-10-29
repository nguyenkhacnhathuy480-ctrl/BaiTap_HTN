#include "board_config.h"
#include "line_follower.h"
#include "line_sensor.h"
#include "motor_tb6612.h"

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

void LineFollower_Init(void)
{
    last_error = 0;
    integral = 0;
}

void LineFollower_Update(void)
{
    int32_t error;
    int32_t derivative;
    int32_t correction;
    int32_t left_speed;
    int32_t right_speed;

    if (LineSensor_GetPosition(&error) == 0U) {
        /* The line has disappeared: turn toward the last observed side. */
        if (last_error < 0) {
            Motor_Set(-LINE_LOST_TURN_SPEED, LINE_LOST_TURN_SPEED);
        } else {
            Motor_Set(LINE_LOST_TURN_SPEED, -LINE_LOST_TURN_SPEED);
        }
        return;
    }

    integral = Clamp(integral + error, -120000, 120000);
    derivative = error - last_error;
    correction = ((LINE_PID_KP * error) +
                  (LINE_PID_KI * integral) +
                  (LINE_PID_KD * derivative)) / 1000;
    correction = Clamp(correction, -LINE_MAX_CORRECTION, LINE_MAX_CORRECTION);

    left_speed = LINE_BASE_SPEED + correction;
    right_speed = LINE_BASE_SPEED - correction;
    left_speed = Clamp(left_speed, -MOTOR_PWM_MAX, MOTOR_PWM_MAX);
    right_speed = Clamp(right_speed, -MOTOR_PWM_MAX, MOTOR_PWM_MAX);

    Motor_Set(left_speed, right_speed);
    last_error = error;
}
