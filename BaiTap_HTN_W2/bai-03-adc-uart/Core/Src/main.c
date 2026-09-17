#include "platform.h"

#include <stdint.h>

static ADC_HandleTypeDef hadc1;
static UART_HandleTypeDef huart1;

static void GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_ADC_CONFIG(RCC_ADCPCLK2_DIV2);

    gpio.Pin = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_10;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio);
}

static void UART1_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200U;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
}

static void ADC1_Init(void)
{
    ADC_ChannelConfTypeDef channel = {0};

    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1U;

    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler();
    }

    channel.Channel = ADC_CHANNEL_0;
    channel.Rank = ADC_REGULAR_RANK_1;
    channel.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;

    if (HAL_ADC_ConfigChannel(&hadc1, &channel) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK) {
        Error_Handler();
    }
}

static void UART_Write(const char *text)
{
    const char *end = text;

    while (*end != '\0') {
        ++end;
    }

    (void)HAL_UART_Transmit(&huart1, (uint8_t *)text,
                            (uint16_t)(end - text), HAL_MAX_DELAY);
}

static void UART_WriteU32(uint32_t value, uint32_t minimum_digits)
{
    char digits[10];
    uint32_t count = 0U;

    do {
        digits[count++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);

    while (count < minimum_digits) {
        digits[count++] = '0';
    }

    while (count != 0U) {
        uint8_t digit = (uint8_t)digits[--count];
        (void)HAL_UART_Transmit(&huart1, &digit, 1U, HAL_MAX_DELAY);
    }
}

int main(void)
{
    uint32_t adc_value;
    uint32_t millivolts;

    Platform_Init();
    GPIO_Init();
    UART1_Init();
    ADC1_Init();

    while (1) {
        if (HAL_ADC_Start(&hadc1) != HAL_OK) {
            Error_Handler();
        }
        if (HAL_ADC_PollForConversion(&hadc1, 10U) != HAL_OK) {
            Error_Handler();
        }

        adc_value = HAL_ADC_GetValue(&hadc1);
        millivolts = (adc_value * 3300U) / 4095U;

        UART_Write("ADC=");
        UART_WriteU32(adc_value, 1U);
        UART_Write(" Voltage=");
        UART_WriteU32(millivolts / 1000U, 1U);
        UART_Write(".");
        UART_WriteU32(millivolts % 1000U, 3U);
        UART_Write(" V\r\n");

        HAL_Delay(1000U);
    }
}
