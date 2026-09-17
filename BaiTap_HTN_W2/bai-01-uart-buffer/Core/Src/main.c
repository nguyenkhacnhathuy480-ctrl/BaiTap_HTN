#include "platform.h"

#include <stdint.h>

#define RX_BUFFER_SIZE 64U

static UART_HandleTypeDef huart1;

static void GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

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

static void UART_Write(const char *text)
{
    const char *end = text;

    while (*end != '\0') {
        ++end;
    }

    (void)HAL_UART_Transmit(&huart1, (uint8_t *)text,
                            (uint16_t)(end - text), HAL_MAX_DELAY);
}

int main(void)
{
    static const char class_and_group[] = "0201: ";
    char buffer[RX_BUFFER_SIZE];
    uint16_t length = 0U;
    uint8_t received;

    Platform_Init();
    GPIO_Init();
    UART1_Init();

    UART_Write("Nhap chuoi va ket thuc bang dau !\r\n");

    while (1) {
        if (HAL_UART_Receive(&huart1, &received, 1U, HAL_MAX_DELAY) != HAL_OK) {
            Error_Handler();
        }

        if (received == (uint8_t)'!') {
            buffer[length] = '\0';
            UART_Write(class_and_group);
            UART_Write(buffer);
            UART_Write("\n\r");
            length = 0U;
        } else if (length < (RX_BUFFER_SIZE - 1U)) {
            buffer[length++] = (char)received;
        }
    }
}
