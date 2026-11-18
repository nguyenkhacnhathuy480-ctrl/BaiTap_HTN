#ifndef OLED_H
#define OLED_H
#include <stdint.h>
#include "i2c_soft.h"
typedef enum { OLED_STAGE_OFF,OLED_STAGE_BUS,OLED_STAGE_PROBE,OLED_STAGE_INIT,
    OLED_STAGE_CLEAR,OLED_STAGE_ON,OLED_STAGE_READY,OLED_STAGE_FAILED } OLEDStage;
typedef struct {
    uint32_t attempts,transactions,failures;
    uint16_t data_index;
    uint8_t scl,sda,address,ack_3c,ack_3d;
    OLEDStage stage;
    I2CSoftError error;
} OLEDDiagnostics;
extern volatile OLEDDiagnostics g_oled;
uint8_t OLED_Init(void);
void OLED_Text(uint8_t row,const char *text);
uint8_t OLED_FlushPage(uint8_t page);
uint8_t OLED_AllPixels(uint8_t on);
extern uint8_t oled_address;
#endif
