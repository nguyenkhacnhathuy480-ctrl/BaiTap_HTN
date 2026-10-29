#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

void Platform_Init(void);
uint32_t Platform_Millis(void);
void Platform_WaitUntil(uint32_t timestamp_ms);
void Platform_StatusLedToggle(void);

#endif
