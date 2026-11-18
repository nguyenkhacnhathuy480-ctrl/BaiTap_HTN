#ifndef I2C_SOFT_H
#define I2C_SOFT_H
#include <stdint.h>
typedef enum {
    I2C_SOFT_OK=0,I2C_SOFT_SCL_LOW,I2C_SOFT_SDA_LOW,
    I2C_SOFT_ADDRESS_NACK,I2C_SOFT_CONTROL_NACK,I2C_SOFT_DATA_NACK
} I2CSoftError;
typedef struct {
    void (*scl)(uint8_t high);
    void (*sda)(uint8_t high);
    uint8_t (*read_scl)(void);
    uint8_t (*read_sda)(void);
    void (*delay_us)(uint32_t us);
} I2CSoftIO;
typedef struct {
    I2CSoftIO io;
    I2CSoftError error;
    uint32_t writes,failures;
    uint16_t data_index;
} I2CSoftBus;
void I2CSoft_Init(I2CSoftBus *bus,const I2CSoftIO *io);
uint8_t I2CSoft_Recover(I2CSoftBus *bus);
uint8_t I2CSoft_Probe(I2CSoftBus *bus,uint8_t address);
uint8_t I2CSoft_Write(I2CSoftBus *bus,uint8_t address,uint8_t control,const uint8_t *data,uint16_t length);
#endif
