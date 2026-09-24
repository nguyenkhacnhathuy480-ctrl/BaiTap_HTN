#include "platform.h"

static void ADC1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_ADC1EN;
    RCC->CFGR &= ~RCC_CFGR_ADCPRE; /* ADC clock = 8 MHz / 2 = 4 MHz. */
    GPIOA->CRL &= ~0xFU;           /* PA0 = analog input. */

    ADC1->CR1 = 0U;
    ADC1->CR2 = 0U;
    ADC1->SQR1 = 0U;
    ADC1->SQR2 = 0U;
    ADC1->SQR3 = 0U;               /* Rank 1 = channel 0. */
    ADC1->SMPR2 = 5U;              /* Channel 0: 55.5 cycles. */

    ADC1->CR2 = ADC_CR2_ADON | ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL;
    Platform_DelayMs(1U);

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

static void UART_WriteMillivolts(uint32_t millivolts)
{
    UART1_WriteU32(millivolts / 1000U);
    UART1_WriteByte((uint8_t)'.');
    UART1_WriteByte((uint8_t)('0' + ((millivolts / 100U) % 10U)));
    UART1_WriteByte((uint8_t)('0' + ((millivolts / 10U) % 10U)));
    UART1_WriteByte((uint8_t)('0' + (millivolts % 10U)));
}

int main(void)
{
    Platform_Init();
    Platform_SysTickStart1ms();
    UART1_Init(115200U);
    ADC1_Init();

    while (1) {
        const uint32_t adc_value = ADC1_Read();
        const uint32_t millivolts = (adc_value * 3300U) / 4095U;

        UART1_WriteString("ADC=");
        UART1_WriteU32(adc_value);
        UART1_WriteString(" Voltage=");
        UART_WriteMillivolts(millivolts);
        UART1_WriteString(" V\r\n");
        Platform_DelayMs(1000U);
    }
}
