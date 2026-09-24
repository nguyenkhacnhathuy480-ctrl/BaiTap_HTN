#ifndef PLATFORM_H
#define PLATFORM_H

#include "stm32f1xx.h"

#include <stdint.h>

#define PLATFORM_CLOCK_HZ 8000000U

void Platform_Init(void);
void Platform_SysTickStart1ms(void);
uint32_t Platform_Millis(void);
void Platform_DelayMs(uint32_t milliseconds);

void UART1_Init(uint32_t baud_rate);
void UART1_WriteByte(uint8_t value);
void UART1_WriteString(const char *text);
void UART1_WriteU32(uint32_t value);
uint8_t UART1_ReadByteBlocking(void);

void App_SysTick_1ms(void);
void Error_Handler(void) __attribute__((noreturn));

#endif
