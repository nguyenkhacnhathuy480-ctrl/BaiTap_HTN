#include "robot.h"
#include "board.h"
#include "line_sensor.h"
#include "motor_driver.h"
#include "uart_debug.h"
#include "user_io.h"

int main(void)
{
    Board_Init();
    Motors_Init();
    UserIO_Init();
    Uart_Init();
    Robot_Start(Sensors_Init());
    return 0;
}
