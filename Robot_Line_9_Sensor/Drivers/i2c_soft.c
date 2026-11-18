/* Transport independent of STM32, testable with simulated SDA/SCL.
 * SDA and SCL callbacks must release high (open drain), never drive high. */
#include "i2c_soft.h"
#include <string.h>
#define HALF_PERIOD_US 10U
#define STRETCH_POLLS 100U
static uint8_t rise(I2CSoftBus *b)
{
    uint32_t count;
    b->io.scl(1U);
    for (count=0U;count<STRETCH_POLLS;++count) {
        if (b->io.read_scl()) { b->io.delay_us(HALF_PERIOD_US);return 1U; }
        b->io.delay_us(1U);
    }
    b->error=I2C_SOFT_SCL_LOW;return 0U;
}
static void stop(I2CSoftBus *b)
{
    b->io.scl(0U);b->io.sda(0U);b->io.delay_us(HALF_PERIOD_US);
    (void)rise(b);b->io.sda(1U);b->io.delay_us(HALF_PERIOD_US);
}
static uint8_t start(I2CSoftBus *b)
{
    b->error=I2C_SOFT_OK;b->data_index=0U;
    b->io.sda(1U);
    if (!rise(b)) return 0U;
    if (!b->io.read_sda()) { b->error=I2C_SOFT_SDA_LOW;return 0U; }
    b->io.sda(0U);b->io.delay_us(HALF_PERIOD_US);b->io.scl(0U);
    return 1U;
}
static uint8_t byte_out(I2CSoftBus *b,uint8_t value,I2CSoftError nack)
{
    uint32_t i;
    for (i=0U;i<8U;++i) {
        b->io.scl(0U);b->io.sda((value & 0x80U)!=0U);
        b->io.delay_us(HALF_PERIOD_US);
        if (!rise(b)) return 0U;
        b->io.scl(0U);value<<=1U;
    }
    b->io.sda(1U);b->io.delay_us(HALF_PERIOD_US);
    if (!rise(b)) return 0U;
    if (b->io.read_sda()) b->error=nack;
    b->io.scl(0U);
    return b->error==I2C_SOFT_OK;
}
void I2CSoft_Init(I2CSoftBus *b,const I2CSoftIO *io)
{
    memset(b,0,sizeof(*b));b->io=*io;
    b->io.sda(1U);b->io.scl(1U);
}
uint8_t I2CSoft_Recover(I2CSoftBus *b)
{
    uint32_t i;
    b->error=I2C_SOFT_OK;b->io.sda(1U);
    if (!rise(b)) return 0U;
    for (i=0U;i<9U && !b->io.read_sda();++i) {
        b->io.scl(0U);b->io.delay_us(HALF_PERIOD_US);
        if (!rise(b)) return 0U;
    }
    stop(b);
    if (b->error==I2C_SOFT_OK && !b->io.read_sda()) b->error=I2C_SOFT_SDA_LOW;
    return b->error==I2C_SOFT_OK;
}
uint8_t I2CSoft_Probe(I2CSoftBus *b,uint8_t address)
{
    uint8_t ok=start(b);
    if (ok) ok=byte_out(b,(uint8_t)(address<<1U),I2C_SOFT_ADDRESS_NACK);
    stop(b);return ok && b->error==I2C_SOFT_OK;
}
uint8_t I2CSoft_Write(I2CSoftBus *b,uint8_t address,uint8_t control,const uint8_t *data,uint16_t length)
{
    uint16_t i;
    uint8_t ok=start(b);
    if (ok) ok=byte_out(b,(uint8_t)(address<<1U),I2C_SOFT_ADDRESS_NACK);
    if (ok) ok=byte_out(b,control,I2C_SOFT_CONTROL_NACK);
    for (i=0U;i<length && ok;++i) {
        b->data_index=i;ok=byte_out(b,data[i],I2C_SOFT_DATA_NACK);
    }
    stop(b);
    ok=ok && b->error==I2C_SOFT_OK;
    if (ok) ++b->writes;else ++b->failures;
    return ok;
}
