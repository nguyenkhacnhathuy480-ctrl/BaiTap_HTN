#include "board_config.h"
#include "line_follower.h"
#include "line_sensor.h"
#include "motor_tb6612.h"
#include "platform.h"

int main(void)
{
    uint32_t next_control_ms;
    uint32_t next_led_ms;

    Platform_Init();
    LineSensor_Init();
    Motor_Init();
    LineFollower_Init();

    next_control_ms = Platform_Millis();
    next_led_ms = next_control_ms + 250U;

    while (1) {
        next_control_ms += CONTROL_PERIOD_MS;
        LineFollower_Update();

        if ((int32_t)(Platform_Millis() - next_led_ms) >= 0) {
            Platform_StatusLedToggle();
            next_led_ms += 250U;
        }
        Platform_WaitUntil(next_control_ms);
    }
}
