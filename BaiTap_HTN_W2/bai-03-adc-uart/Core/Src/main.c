#include "stm32f1xx.h"

#include <stdint.h>

#define SYSTEM_CLOCK_HZ 8000000U

static volatile uint32_t milliseconds;

void SysTick_Handler(void)
{
    ++milliseconds;
}

static void Clock_Init(void)
{
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0U) {
    }
    RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE |
                   RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
    RCC->CFGR |= RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) {
    }
    SystemCoreClock = SYSTEM_CLOCK_HZ;
}

static void DelayMs(uint32_t delay)
{
    const uint32_t start = milliseconds;

    while ((milliseconds - start) < delay) {
        __WFI();
    }
}

static void UART1_Init(uint32_t baud_rate)
{
    uint32_t crh;

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    crh = GPIOA->CRH;
    crh &= ~((0xFU << 4U) | (0xFU << 8U));
    crh |= (0xBU << 4U) | (0x4U << 8U);
    GPIOA->CRH = crh;

    USART1->BRR = (SYSTEM_CLOCK_HZ + (baud_rate / 2U)) / baud_rate;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

static void UART1_WriteByte(uint8_t value)
{
    while ((USART1->SR & USART_SR_TXE) == 0U) {
    }
    USART1->DR = value;
}

static void UART1_WriteString(const char *text)
{
    while (*text != '\0') {
        UART1_WriteByte((uint8_t)*text++);
    }
}

static void UART1_WriteU32(uint32_t value)
{
    char digits[10];
    uint32_t count = 0U;

    do {
        digits[count++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);

    while (count != 0U) {
        UART1_WriteByte((uint8_t)digits[--count]);
    }
}

static void ADC1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_ADC1EN;
    RCC->CFGR &= ~RCC_CFGR_ADCPRE; /* ADC clock = 8 MHz / 2 = 4 MHz. */
    GPIOA->CRL &= ~0xFU;           /* PA0 = analog input. */

    ADC1->CR1 = 0U;
    ADC1->CR2 = 0U;
    ADC1->SQR1 = 0U;
    ADC1->SQR2 = 0U;
    ADC1->SQR3 = 0U;              /* Rank 1 = channel 0. */
    ADC1->SMPR2 = 5U;             /* Channel 0: 55.5 cycles. */

    ADC1->CR2 = ADC_CR2_ADON | ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL;
    DelayMs(1U);
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while ((ADC1->CR2 & ADC_CR2_RSTCAL) != 0U) {
    }
    ADC1->CR2 |= ADC_CR2_CAL;
    while ((ADC1->CR2 & ADC_CR2_CAL) != 0U) {
    }
}

static uint16_t ADC1_Read(void)
{
    ADC1->SR = 0U;
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while ((ADC1->SR & ADC_SR_EOC) == 0U) {
    }
    return (uint16_t)ADC1->DR;
}

static void UART1_WriteMillivolts(uint32_t millivolts)
{
    UART1_WriteU32(millivolts / 1000U);
    UART1_WriteByte((uint8_t)'.');
    UART1_WriteByte((uint8_t)('0' + ((millivolts / 100U) % 10U)));
    UART1_WriteByte((uint8_t)('0' + ((millivolts / 10U) % 10U)));
    UART1_WriteByte((uint8_t)('0' + (millivolts % 10U)));
}

int main(void)
{
    Clock_Init();
    if (SysTick_Config(SYSTEM_CLOCK_HZ / 1000U) != 0U) {
        while (1) {
        }
    }
    UART1_Init(115200U);
    ADC1_Init();

    while (1) {
        const uint32_t adc_value = ADC1_Read();
        const uint32_t millivolts = (adc_value * 3300U) / 4095U;

        UART1_WriteString("ADC=");
        UART1_WriteU32(adc_value);
        UART1_WriteString(" Voltage=");
        UART1_WriteMillivolts(millivolts);
        UART1_WriteString(" V\r\n");
        DelayMs(1000U);
    }
}
