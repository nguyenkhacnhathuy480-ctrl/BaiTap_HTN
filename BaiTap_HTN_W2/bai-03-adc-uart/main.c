#include <stdint.h>

#define RCC_CFGR    (*(volatile uint32_t *)0x40021004u)
#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018u)
#define GPIOA_CRL   (*(volatile uint32_t *)0x40010800u)
#define GPIOA_CRH   (*(volatile uint32_t *)0x40010804u)

#define USART1_SR   (*(volatile uint32_t *)0x40013800u)
#define USART1_DR   (*(volatile uint32_t *)0x40013804u)
#define USART1_BRR  (*(volatile uint32_t *)0x40013808u)
#define USART1_CR1  (*(volatile uint32_t *)0x4001380Cu)

#define ADC1_SR     (*(volatile uint32_t *)0x40012400u)
#define ADC1_CR2    (*(volatile uint32_t *)0x40012408u)
#define ADC1_SMPR2  (*(volatile uint32_t *)0x40012410u)
#define ADC1_SQR1   (*(volatile uint32_t *)0x4001242Cu)
#define ADC1_SQR3   (*(volatile uint32_t *)0x40012434u)
#define ADC1_DR     (*(volatile uint32_t *)0x4001244Cu)

#define SYST_CSR    (*(volatile uint32_t *)0xE000E010u)
#define SYST_RVR    (*(volatile uint32_t *)0xE000E014u)
#define SYST_CVR    (*(volatile uint32_t *)0xE000E018u)

#define USART_SR_TXE (1u << 7)

static void uart_write_char(char value)
{
    while ((USART1_SR & USART_SR_TXE) == 0u) {
    }
    USART1_DR = (uint32_t)(uint8_t)value;
}

static void uart_write_string(const char *text)
{
    while (*text != '\0') {
        uart_write_char(*text++);
    }
}

static void uart_write_uint(uint32_t value)
{
    char digits[10];
    uint32_t count = 0u;

    if (value == 0u) {
        uart_write_char('0');
        return;
    }

    while (value != 0u) {
        digits[count++] = (char)('0' + (value % 10u));
        value /= 10u;
    }

    while (count != 0u) {
        uart_write_char(digits[--count]);
    }
}

static void uart_write_3digits(uint32_t value)
{
    uart_write_char((char)('0' + ((value / 100u) % 10u)));
    uart_write_char((char)('0' + ((value / 10u) % 10u)));
    uart_write_char((char)('0' + (value % 10u)));
}

static void uart_init(void)
{
    /* PA9: alternate push-pull 50 MHz; PA10: floating input. */
    GPIOA_CRH = (GPIOA_CRH & ~((0xFu << 4) | (0xFu << 8))) |
                (0xBu << 4) | (0x4u << 8);
    USART1_BRR = 0x45u;
    USART1_CR1 = (1u << 13) | (1u << 3) | (1u << 2);
}

static void adc_init(void)
{
    /* PCLK2/2 = 4 MHz ADC clock and PA0 in analog-input mode. */
    RCC_CFGR &= ~(0x3u << 14);
    GPIOA_CRL &= ~0xFu;

    ADC1_SMPR2 = (ADC1_SMPR2 & ~0x7u) | 0x7u;
    ADC1_SQR1 = 0u;
    ADC1_SQR3 = 0u;

    ADC1_CR2 = (1u << 0);
    for (volatile uint32_t delay = 0u; delay < 1000u; ++delay) {
    }

    ADC1_CR2 |= (1u << 3);
    while ((ADC1_CR2 & (1u << 3)) != 0u) {
    }

    ADC1_CR2 |= (1u << 2);
    while ((ADC1_CR2 & (1u << 2)) != 0u) {
    }

    ADC1_CR2 |= (0x7u << 17) | (1u << 20);
}

static uint32_t adc_read_channel0(void)
{
    ADC1_CR2 |= (1u << 22);
    while ((ADC1_SR & (1u << 1)) == 0u) {
    }
    return ADC1_DR & 0xFFFu;
}

static void systick_init(void)
{
    SYST_RVR = 8000u - 1u;
    SYST_CVR = 0u;
    SYST_CSR = (1u << 2) | (1u << 0);
}

static void delay_ms(uint32_t milliseconds)
{
    while (milliseconds-- != 0u) {
        while ((SYST_CSR & (1u << 16)) == 0u) {
        }
    }
}

int main(void)
{
    RCC_APB2ENR |= (1u << 0) | (1u << 2) | (1u << 9) | (1u << 14);
    uart_init();
    adc_init();
    systick_init();

    for (;;) {
        uint32_t raw = adc_read_channel0();
        uint32_t millivolts = ((raw * 3300u) + 2047u) / 4095u;

        uart_write_string("ADC=");
        uart_write_uint(raw);
        uart_write_string(" Voltage=");
        uart_write_uint(millivolts / 1000u);
        uart_write_char('.');
        uart_write_3digits(millivolts % 1000u);
        uart_write_string(" V\r\n");

        delay_ms(1000u);
    }
}
