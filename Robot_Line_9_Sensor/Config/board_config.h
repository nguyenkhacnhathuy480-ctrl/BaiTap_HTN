#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H
#include "stm32f1xx.h"
#define BOARD_CPU_HZ 72000000U
#define BOARD_PWM_HZ 20000U
#include "sensor_map.h"
#define BOARD_OLED_ADDRESS_FIRST 0x3CU
#if BOARD_OLED_ADDRESS_FIRST != 0x3CU && BOARD_OLED_ADDRESS_FIRST != 0x3DU
#error Invalid_OLED_7bit_address
#endif
/* 1315 = DM-OLED096-636 manufacturer profile; 1306 for a verified substitute.
 * An I2C ACK cannot identify the display controller. */
#define BOARD_OLED_CONTROLLER 1315
/* The separate OLED_Diagnostic project overrides this to 1.
 * Normal RobotLine builds must always include the full robot application. */
#ifndef BOARD_OLED_TEST_ONLY
#define BOARD_OLED_TEST_ONLY 0U
#endif
#ifndef BOARD_MOTOR_TEST_ONLY
#define BOARD_MOTOR_TEST_ONLY 0U
#endif
#ifndef BOARD_FAST_MOTOR_TEST
#define BOARD_FAST_MOTOR_TEST 0U
#endif
#if BOARD_FAST_MOTOR_TEST && !BOARD_MOTOR_TEST_ONLY
#error Fast_test_requires_motor_test_mode
#endif
#if BOARD_OLED_TEST_ONLY && BOARD_MOTOR_TEST_ONLY
#error Select_only_one_diagnostic_mode
#endif
#define BOARD_BUZZER_PASSIVE 0U
/* Reverse both outputs relative to the previous L=0/R=1 build:
 * user reports both wheels driving backward. Recheck with wheels raised. */
#define BOARD_LEFT_MOTOR_REVERSED 1U
#define BOARD_RIGHT_MOTOR_REVERSED 0U
#define BOARD_LED_PORT GPIOC
#define BOARD_LED_PIN 13U
#define BOARD_STBY_PORT GPIOA
#define BOARD_STBY_PIN 10U
#define BOARD_LEFT_IN1_PORT GPIOB
#define BOARD_LEFT_IN1_PIN 12U
#define BOARD_LEFT_IN2_PORT GPIOB
#define BOARD_LEFT_IN2_PIN 13U
#define BOARD_RIGHT_IN1_PORT GPIOB
#define BOARD_RIGHT_IN1_PIN 14U
#define BOARD_RIGHT_IN2_PORT GPIOB
#define BOARD_RIGHT_IN2_PIN 15U
/* ADC PA0..7/PB0; PWM PA8/PA9; STBY PA10; OLED PB6/PB7;
 * UART3 PB10/PB11; buttons PB5/PB8/PB9; buzzer PB1. HSE = 8 MHz. */
#endif
