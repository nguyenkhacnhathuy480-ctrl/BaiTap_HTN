#include <stdint.h>

/* =========================================================
   STM32F103 REGISTER ADDRESS
   ========================================================= */

/* RCC */
#define RCC_BASE        0x40021000UL
#define RCC_CR          (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_CFGR        (*(volatile uint32_t *)(RCC_BASE + 0x04))
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x1C))

/* GPIOA */
#define GPIOA_BASE      0x40010800UL
#define GPIOA_CRL       (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_CRH       (*(volatile uint32_t *)(GPIOA_BASE + 0x04))

/* GPIOB */
#define GPIOB_BASE      0x40010C00UL
#define GPIOB_CRL       (*(volatile uint32_t *)(GPIOB_BASE + 0x00))

/* I2C1 */
#define I2C1_BASE       0x40005400UL

#define I2C1_CR1        (*(volatile uint32_t *)(I2C1_BASE + 0x00))
#define I2C1_CR2        (*(volatile uint32_t *)(I2C1_BASE + 0x04))
#define I2C1_OAR1       (*(volatile uint32_t *)(I2C1_BASE + 0x08))
#define I2C1_DR         (*(volatile uint32_t *)(I2C1_BASE + 0x10))
#define I2C1_SR1        (*(volatile uint32_t *)(I2C1_BASE + 0x14))
#define I2C1_SR2        (*(volatile uint32_t *)(I2C1_BASE + 0x18))
#define I2C1_CCR        (*(volatile uint32_t *)(I2C1_BASE + 0x1C))
#define I2C1_TRISE      (*(volatile uint32_t *)(I2C1_BASE + 0x20))

/* USART1 */
#define USART1_BASE     0x40013800UL

#define USART1_SR       (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_DR       (*(volatile uint32_t *)(USART1_BASE + 0x04))
#define USART1_BRR      (*(volatile uint32_t *)(USART1_BASE + 0x08))
#define USART1_CR1      (*(volatile uint32_t *)(USART1_BASE + 0x0C))
#define USART1_CR2      (*(volatile uint32_t *)(USART1_BASE + 0x10))
#define USART1_CR3      (*(volatile uint32_t *)(USART1_BASE + 0x14))


/* =========================================================
   BMP280
   ========================================================= */

#define BMP280_ADDR     0x76

#define BMP280_ID       0xD0
#define BMP280_RESET    0xE0
#define BMP280_CTRL     0xF4
#define BMP280_CONFIG   0xF5
#define BMP280_DATA     0xF7


/* =========================================================
   DELAY
   ========================================================= */

static void delay(volatile uint32_t time)
{
    while (time--)
    {
        __asm volatile ("nop");
    }
}


/* =========================================================
   CLOCK 72 MHz
   HSE 8 MHz -> PLL x9
   ========================================================= */

static void Clock_Init(void)
{
    /* HSE ON */
    RCC_CR |= (1 << 16);

    while (!(RCC_CR & (1 << 17)))
    {
    }

    /* PLL source = HSE
       PLL = x9
    */
    RCC_CFGR &= ~(0xF << 18);
    RCC_CFGR |=  (0x7 << 18);

    RCC_CFGR &= ~(1 << 16);
    RCC_CFGR |=  (1 << 16);

    /* APB1 = HCLK / 2 */
    RCC_CFGR &= ~(0x7 << 8);
    RCC_CFGR |=  (0x4 << 8);

    /* APB2 = HCLK */
    RCC_CFGR &= ~(0x7 << 11);

    /* PLL ON */
    RCC_CR |= (1 << 24);

    while (!(RCC_CR & (1 << 25)))
    {
    }

    /* SYSCLK = PLL */
    RCC_CFGR &= ~(0x3);
    RCC_CFGR |=  (0x2);

    while (((RCC_CFGR >> 2) & 0x3) != 0x2)
    {
    }
}


/* =========================================================
   USART1
   PA9  = TX
   PA10 = RX
   115200 baud
   ========================================================= */

static void USART1_Init(void)
{
    /* Enable GPIOA + USART1 */
    RCC_APB2ENR |= (1 << 2);
    RCC_APB2ENR |= (1 << 14);

    /*
       PA9:
       MODE9 = 11
       CNF9  = 10
       -> AF Push Pull, 50 MHz

       PA10:
       MODE10 = 00
       CNF10  = 01
       -> Input floating
    */

    GPIOA_CRH &= ~((0xF << 4) | (0xF << 8));

    GPIOA_CRH |= (0xB << 4);
    GPIOA_CRH |= (0x4 << 8);

    /*
       Baudrate:
       72 MHz / 115200 = 625

       BRR = 0x0271
    */

    USART1_BRR = 0x0271;

    /* 8 bit, no parity */
    USART1_CR1 = 0;

    /* UE = 1
       TE = 1
       RE = 1
    */
    USART1_CR1 |= (1 << 13);
    USART1_CR1 |= (1 << 3);
    USART1_CR1 |= (1 << 2);

    USART1_CR2 = 0;
    USART1_CR3 = 0;
}


/* =========================================================
   UART SEND BYTE
   ========================================================= */

static void USART1_SendChar(char c)
{
    while (!(USART1_SR & (1 << 7)))
    {
    }

    USART1_DR = c;
}


/* =========================================================
   UART SEND STRING
   ========================================================= */

static void USART1_SendString(const char *str)
{
    while (*str)
    {
        USART1_SendChar(*str++);
    }
}


/* =========================================================
   UART SEND NUMBER
   ========================================================= */

static void UART_SendUInt(uint32_t number)
{
    char buffer[10];
    int i = 0;

    if (number == 0)
    {
        USART1_SendChar('0');
        return;
    }

    while (number > 0)
    {
        buffer[i++] = '0' + (number % 10);
        number /= 10;
    }

    while (i > 0)
    {
        USART1_SendChar(buffer[--i]);
    }
}


/* =========================================================
   I2C1
   PB6 = SCL
   PB7 = SDA
   100 kHz
   ========================================================= */

static void I2C1_Init(void)
{
    /* GPIOB clock */
    RCC_APB2ENR |= (1 << 3);

    /* I2C1 clock */
    RCC_APB1ENR |= (1 << 21);

    /*
       PB6:
       MODE6 = 11
       CNF6  = 11
       AF Open Drain

       PB7:
       MODE7 = 11
       CNF7  = 11
       AF Open Drain
    */

    GPIOB_CRL &= ~((0xF << 24) | (0xF << 28));

    GPIOB_CRL |= (0xF << 24);
    GPIOB_CRL |= (0xF << 28);

    /* I2C peripheral clock = 36 MHz */
    I2C1_CR2 = 36;

    /* Own address */
    I2C1_OAR1 = (1 << 14);

    /*
       100 kHz:

       CCR = 36MHz / (2 * 100kHz)
           = 180
    */
    I2C1_CCR = 180;

    /* TRISE = 36 + 1 */
    I2C1_TRISE = 37;

    /* Enable I2C */
    I2C1_CR1 = (1 << 0);

    delay(10000);
}


/* =========================================================
   I2C START
   ========================================================= */

static uint8_t I2C1_Start(void)
{
    uint32_t timeout = 100000;

    I2C1_CR1 |= (1 << 8);

    while (!(I2C1_SR1 & (1 << 0)))
    {
        if (--timeout == 0)
            return 0;
    }

    return 1;
}


/* =========================================================
   I2C STOP
   ========================================================= */

static void I2C1_Stop(void)
{
    I2C1_CR1 |= (1 << 9);
}


/* =========================================================
   I2C SEND BYTE
   ========================================================= */

static uint8_t I2C1_SendByte(uint8_t data)
{
    uint32_t timeout = 100000;

    I2C1_DR = data;

    while (!(I2C1_SR1 & (1 << 7)))
    {
        if (--timeout == 0)
            return 0;
    }

    return 1;
}


/* =========================================================
   I2C SEND ADDRESS
   ========================================================= */

static uint8_t I2C1_SendAddress(uint8_t address)
{
    uint32_t timeout = 100000;

    I2C1_DR = address;

    while (!(I2C1_SR1 & (1 << 1)))
    {
        if (--timeout == 0)
            return 0;
    }

    /* Clear ADDR */
    (void)I2C1_SR1;
    (void)I2C1_SR2;

    return 1;
}


/* =========================================================
   I2C READ BYTE ACK
   ========================================================= */

static uint8_t I2C1_ReadByte_ACK(void)
{
    I2C1_CR1 |= (1 << 10);

    while (!(I2C1_SR1 & (1 << 6)))
    {
    }

    return (uint8_t)I2C1_DR;
}


/* =========================================================
   I2C READ BYTE NACK
   ========================================================= */

static uint8_t I2C1_ReadByte_NACK(void)
{
    uint8_t data;

    I2C1_CR1 &= ~(1 << 10);

    while (!(I2C1_SR1 & (1 << 6)))
    {
    }

    data = (uint8_t)I2C1_DR;

    return data;
}


/* =========================================================
   BMP280 WRITE REGISTER
   ========================================================= */

static uint8_t BMP280_WriteReg(
    uint8_t reg,
    uint8_t value)
{
    if (!I2C1_Start())
        return 0;

    if (!I2C1_SendAddress(
            (BMP280_ADDR << 1) | 0))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_SendByte(reg))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_SendByte(value))
    {
        I2C1_Stop();
        return 0;
    }

    I2C1_Stop();

    return 1;
}


/* =========================================================
   BMP280 READ REG
   ========================================================= */

static uint8_t BMP280_ReadReg(uint8_t reg)
{
    uint8_t data;

    if (!I2C1_Start())
        return 0;

    if (!I2C1_SendAddress(
            (BMP280_ADDR << 1) | 0))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_SendByte(reg))
    {
        I2C1_Stop();
        return 0;
    }

    /* Repeated START */
    if (!I2C1_Start())
        return 0;

    if (!I2C1_SendAddress(
            (BMP280_ADDR << 1) | 1))
    {
        I2C1_Stop();
        return 0;
    }

    data = I2C1_ReadByte_NACK();

    I2C1_Stop();

    return data;
}


/* =========================================================
   BMP280 READ 6 BYTES
   ========================================================= */

static uint8_t BMP280_ReadData(uint8_t *data)
{
    uint8_t i;

    if (!I2C1_Start())
        return 0;

    if (!I2C1_SendAddress(
            (BMP280_ADDR << 1) | 0))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_SendByte(0xF7))
    {
        I2C1_Stop();
        return 0;
    }

    /* Repeated START */
    if (!I2C1_Start())
        return 0;

    if (!I2C1_SendAddress(
            (BMP280_ADDR << 1) | 1))
    {
        I2C1_Stop();
        return 0;
    }

    for (i = 0; i < 5; i++)
    {
        data[i] = I2C1_ReadByte_ACK();
    }

    data[5] = I2C1_ReadByte_NACK();

    I2C1_Stop();

    return 1;
}


/* =========================================================
   BMP280 INIT
   ========================================================= */

static uint8_t BMP280_Init(void)
{
    uint8_t id;

    id = BMP280_ReadReg(BMP280_ID);

    if (id != 0x58)
    {
        return 0;
    }

    /* Reset */
    BMP280_WriteReg(
        BMP280_RESET,
        0xB6
    );

    delay(100000);

    /*
       Temperature x1
       Pressure x1
       Normal mode
    */

    BMP280_WriteReg(
        BMP280_CTRL,
        0x27
    );

    /*
       Standby 1000 ms
       Filter OFF
    */

    BMP280_WriteReg(
        BMP280_CONFIG,
        0xA0
    );

    return 1;
}


/* =========================================================
   MAIN
   ========================================================= */

int main(void)
{
    uint8_t id;
    uint8_t data[6];

    uint32_t pressure_raw;
    uint32_t temperature_raw;

    Clock_Init();

    USART1_Init();

    I2C1_Init();

    USART1_SendString(
        "\r\nSTM32F103 BMP280\r\n"
    );

    USART1_SendString(
        "UART OK\r\n"
    );

    /* Đọc CHIP ID */

    id = BMP280_ReadReg(BMP280_ID);

    USART1_SendString(
        "BMP280 ID = 0x"
    );

    if (id == 0x58)
    {
        USART1_SendString("58\r\n");
    }
    else
    {
        USART1_SendString("ERROR\r\n");
    }

    /* Khởi tạo BMP280 */

    if (!BMP280_Init())
    {
        USART1_SendString(
            "BMP280 INIT ERROR\r\n"
        );

        while (1)
        {
        }
    }

    USART1_SendString(
        "BMP280 OK\r\n"
    );

    while (1)
    {
        if (BMP280_ReadData(data))
        {
            pressure_raw =
                ((uint32_t)data[0] << 12) |
                ((uint32_t)data[1] << 4) |
                (data[2] >> 4);

            temperature_raw =
                ((uint32_t)data[3] << 12) |
                ((uint32_t)data[4] << 4) |
                (data[5] >> 4);

            USART1_SendString(
                "RAW P = "
            );

            UART_SendUInt(
                pressure_raw
            );

            USART1_SendString(
                "   RAW T = "
            );

            UART_SendUInt(
                temperature_raw
            );

            USART1_SendString(
                "\r\n"
            );
        }
        else
        {
            USART1_SendString(
                "I2C READ ERROR\r\n"
            );
        }

        delay(5000000);
    }
}