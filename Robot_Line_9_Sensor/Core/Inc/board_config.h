#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/*
 * Central hardware configuration.
 * Change pin mapping and control constants here when rewiring the robot.
 */

#include "stm32f1xx.h"

#include <stdint.h>

#define BOARD_SYSTEM_CLOCK_HZ       72000000U
#define CONTROL_PERIOD_MS           1U
#define LINE_SENSOR_COUNT           9U

/* Digital line sensors S0 (far left) through S8 (far right). */
#define LINE_SENSOR_ACTIVE_LOW      1U
#define LINE_SENSOR_PORT_A_MASK     0x00FFU /* S0..S7 = PA0..PA7 */
#define LINE_SENSOR_PORT_B_PIN      0U      /* S8 = PB0 */

/* TB6612FNG connection. PWM is produced by TIM1 on PA8 and PA9. */
#define MOTOR_PWM_FREQUENCY_HZ      20000U
#define MOTOR_PWM_MAX               1000

#define TB6612_STBY_PORT            GPIOB
#define TB6612_STBY_PIN             11U

#define TB6612_AIN1_PORT            GPIOB
#define TB6612_AIN1_PIN             12U
#define TB6612_AIN2_PORT            GPIOB
#define TB6612_AIN2_PIN             13U

#define TB6612_BIN1_PORT            GPIOB
#define TB6612_BIN1_PIN             14U
#define TB6612_BIN2_PORT            GPIOB
#define TB6612_BIN2_PIN             15U

/* PC13 is the on-board LED on most Blue Pill boards (active low). */
#define STATUS_LED_PORT             GPIOC
#define STATUS_LED_PIN              13U

/*
 * PID gains use scale 1000:
 * output = (KP * error + KI * integral + KD * derivative) / 1000.
 * Tune these four values for the actual chassis and track.
 */
#define LINE_PID_KP                 170
#define LINE_PID_KI                 2
#define LINE_PID_KD                 280
#define LINE_BASE_SPEED             520
#define LINE_MAX_CORRECTION         850
#define LINE_LOST_TURN_SPEED        320

#endif
