#include "platform.h"

#include <stdint.h>

#define COMMAND_BUFFER_SIZE 32U

static TIM_HandleTypeDef htim2;
static UART_HandleTypeDef huart1;

static volatile uint8_t received_byte;
static volatile char command_buffer[COMMAND_BUFFER_SIZE];
static volatile uint32_t command_length;
static volatile uint32_t command_ready;

static uint32_t led_enabled;
static uint32_t pwm_percent = 50U;

static void GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_10;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio);
}

static void TIM2_PWM_Init(void)
{
    TIM_OC_InitTypeDef pwm = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7U;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999U;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }

    pwm.OCMode = TIM_OCMODE_PWM1;
    pwm.Pulse = 0U;
    pwm.OCPolarity = TIM_OCPOLARITY_HIGH;
    pwm.OCFastMode = TIM_OCFAST_DISABLE;

    if ((HAL_TIM_PWM_ConfigChannel(&htim2, &pwm, TIM_CHANNEL_1) != HAL_OK) ||
        (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1) != HAL_OK)) {
        Error_Handler();
    }
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

    HAL_NVIC_SetPriority(USART1_IRQn, 1U, 0U);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    if (HAL_UART_Receive_IT(&huart1, (uint8_t *)&received_byte, 1U) != HAL_OK) {
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

static void UART_WriteU32(uint32_t value)
{
    char digits[10];
    uint32_t count = 0U;

    do {
        digits[count++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);

    while (count != 0U) {
        uint8_t digit = (uint8_t)digits[--count];
        (void)HAL_UART_Transmit(&huart1, &digit, 1U, HAL_MAX_DELAY);
    }
}

static char ToUpper(char value)
{
    if ((value >= 'a') && (value <= 'z')) {
        value = (char)(value - ('a' - 'A'));
    }
    return value;
}

static uint32_t StringEqualsIgnoreCase(const char *left, const char *right)
{
    while ((*left != '\0') && (*right != '\0')) {
        if (ToUpper(*left++) != ToUpper(*right++)) {
            return 0U;
        }
    }
    return ((*left == '\0') && (*right == '\0')) ? 1U : 0U;
}

static uint32_t ParsePWM(const char *text, uint32_t *result)
{
    uint32_t index = 4U;
    uint32_t value = 0U;
    uint32_t has_digit = 0U;

    if ((ToUpper(text[0]) != 'P') || (ToUpper(text[1]) != 'W') ||
        (ToUpper(text[2]) != 'M') || (text[3] != ':')) {
        return 0U;
    }

    while ((text[index] >= '0') && (text[index] <= '9')) {
        has_digit = 1U;
        value = (value * 10U) + (uint32_t)(text[index] - '0');
        ++index;
    }

    if (text[index] == '%') {
        ++index;
    }

    if ((has_digit == 0U) || (text[index] != '\0') || (value > 100U)) {
        return 0U;
    }

    *result = value;
    return 1U;
}

static void ApplyPWM(void)
{
    uint32_t pulse = (led_enabled != 0U) ? (pwm_percent * 10U) : 0U;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);
}

static void SendStatus(void)
{
    UART_Write("STATUS:");
    UART_Write((led_enabled != 0U) ? "ON" : "OFF");
    UART_Write(" PWM:");
    UART_WriteU32(pwm_percent);
    UART_Write("%\r\n");
}

static void ProcessCommand(const char *command)
{
    uint32_t new_percent;

    if (StringEqualsIgnoreCase(command, "ON") != 0U) {
        led_enabled = 1U;
        ApplyPWM();
        UART_Write("OK ON\r\n");
    } else if (StringEqualsIgnoreCase(command, "OFF") != 0U) {
        led_enabled = 0U;
        ApplyPWM();
        UART_Write("OK OFF\r\n");
    } else if (StringEqualsIgnoreCase(command, "STATUS") != 0U) {
        SendStatus();
    } else if (ParsePWM(command, &new_percent) != 0U) {
        pwm_percent = new_percent;
        ApplyPWM();
        UART_Write("OK PWM:");
        UART_WriteU32(pwm_percent);
        UART_Write("%\r\n");
    } else {
        UART_Write("ERROR: LENH KHONG HOP LE\r\n");
    }
}

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart)
{
    if (uart->Instance == USART1) {
        if (command_ready == 0U) {
            if (received_byte == (uint8_t)'!') {
                command_buffer[command_length] = '\0';
                command_ready = 1U;
            } else if (command_length < (COMMAND_BUFFER_SIZE - 1U)) {
                command_buffer[command_length++] = (char)received_byte;
            }
        }

        (void)HAL_UART_Receive_IT(&huart1, (uint8_t *)&received_byte, 1U);
    }
}

int main(void)
{
    Platform_Init();
    GPIO_Init();
    TIM2_PWM_Init();
    UART1_Init();

    UART_Write("Lenh: ON! OFF! PWM:0..100%! STATUS!\r\n");

    while (1) {
        if (command_ready != 0U) {
            ProcessCommand((const char *)command_buffer);
            command_length = 0U;
            command_ready = 0U;
        }
    }
}
