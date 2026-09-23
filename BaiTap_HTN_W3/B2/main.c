#include <stdint.h>

/* =========================
   RCC
   ========================= */

#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018)

/* =========================
   GPIOA
   ========================= */

#define GPIOA_CRL   (*(volatile uint32_t *)0x40010800)
#define GPIOA_ODR   (*(volatile uint32_t *)0x4001080C)

/* =========================
   SPI1
   ========================= */

#define SPI1_CR1    (*(volatile uint32_t *)0x40013000)
#define SPI1_SR     (*(volatile uint32_t *)0x40013008)
#define SPI1_DR     (*(volatile uint32_t *)0x4001300C)

/* =========================
   SysTick
   ========================= */

#define SYST_CSR    (*(volatile uint32_t *)0xE000E010)
#define SYST_RVR    (*(volatile uint32_t *)0xE000E014)
#define SYST_CVR    (*(volatile uint32_t *)0xE000E018)


/* CS = PA4 */

#define CS_LOW()    (GPIOA_ODR &= ~(1 << 4))
#define CS_HIGH()   (GPIOA_ODR |=  (1 << 4))


/* =========================
   SysTick
   ========================= */

void SysTick_Init(void)
{
    /*
       HSI = 8 MHz

       1 ms:
       8,000,000 / 1000 = 8000
    */

    SYST_RVR = 7999;
    SYST_CVR = 0;

    SYST_CSR = 0x05;
}


void delay_ms(uint32_t ms)
{
    uint32_t i;

    for(i = 0; i < ms; i++)
    {
        while((SYST_CSR & 0x10000) == 0);
    }
}


/* =========================
   GPIOA
   ========================= */

void GPIO_Init(void)
{
    /*
       Enable GPIOA clock
    */

    RCC_APB2ENR |= (1 << 2);

    /*
       PA4 = Output Push-Pull
       PA5 = SPI1 SCK
       PA7 = SPI1 MOSI
    */

    GPIOA_CRL &= ~(
        (0xF << 16) |
        (0xF << 20) |
        (0xF << 28)
    );

    /*
       PA4 = 0011
       PA5 = 1011
       PA7 = 1011
    */

    GPIOA_CRL |= (
        (0x3 << 16) |
        (0xB << 20) |
        (0xB << 28)
    );

    CS_HIGH();
}


/* =========================
   SPI1
   ========================= */

void SPI1_Init(void)
{
    /*
       Enable SPI1 clock
       APB2ENR bit 12
    */

    RCC_APB2ENR |= (1 << 12);

    SPI1_CR1 = 0;

    /*
       Master
       Software NSS
       Internal NSS high
       MSB first
       CPOL = 0
       CPHA = 0
    */

    SPI1_CR1 |= (1 << 2);
    SPI1_CR1 |= (1 << 9);
    SPI1_CR1 |= (1 << 8);

    /*
       SPI enable
    */

    SPI1_CR1 |= (1 << 6);
}


/* =========================
   SPI gửi 1 byte
   ========================= */

void SPI1_SendByte(uint8_t data)
{
    /*
       Chờ TXE
    */

    while((SPI1_SR & (1 << 1)) == 0);

    SPI1_DR = data;

    /*
       Chờ truyền xong
    */

    while(SPI1_SR & (1 << 7));
}


/* =========================
   MAX7219
   ========================= */

void MAX7219_Send(uint8_t address, uint8_t data)
{
    CS_LOW();

    SPI1_SendByte(address);
    SPI1_SendByte(data);

    CS_HIGH();
}


/* =========================
   MAX7219 Init
   ========================= */

void MAX7219_Init(void)
{
    /*
       Display Test OFF
    */

    MAX7219_Send(0x0F, 0x00);

    /*
       Normal operation
    */

    MAX7219_Send(0x0C, 0x01);

    /*
       Scan 8 rows
    */

    MAX7219_Send(0x0B, 0x07);

    /*
       Không dùng giải mã 7 đoạn
       Vì đang dùng ma trận LED
    */

    MAX7219_Send(0x09, 0x00);

    /*
       Độ sáng
    */

    MAX7219_Send(0x0A, 0x08);

    /*
       Xóa toàn bộ ma trận
    */

    for(uint8_t i = 1; i <= 8; i++)
    {
        MAX7219_Send(i, 0x00);
    }
}


/* =========================
   Hiển thị ma trận
   ========================= */

void Matrix_Display(const uint8_t *data)
{
    uint8_t row;

    for(row = 0; row < 8; row++)
    {
        MAX7219_Send(row + 1, data[row]);
    }
}


/* =========================
   Chữ A
   ========================= */

const uint8_t letter_A[8] =
{
    0x18,
    0x24,
    0x42,
    0x42,
    0x7E,
    0x42,
    0x42,
    0x00
};


/* =========================
   Main
   ========================= */

int main(void)
{
    GPIO_Init();

    SPI1_Init();

    SysTick_Init();

    MAX7219_Init();

    while(1)
    {
        Matrix_Display(letter_A);

        delay_ms(500);
    }
}