#ifndef BOARD_H
#define BOARD_H
#include "board_config.h"
void Board_Init(void);
void Board_Output(GPIO_TypeDef *port, uint32_t pin);
void Board_Write(GPIO_TypeDef *port, uint32_t pin, uint8_t high);
void Board_LedToggle(void);
void Board_EmergencyStop(void);
void Board_DelayUs(uint32_t us);
#endif
