#include "platform.h"

static TIM_HandleTypeDef htim2;

static void GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);
}

static void PWM_Channel_Init(uint32_t channel, uint32_t pulse)
{
    TIM_OC_InitTypeDef pwm = {0};

    pwm.OCMode = TIM_OCMODE_PWM1;
    pwm.Pulse = pulse;
    pwm.OCPolarity = TIM_OCPOLARITY_HIGH;
    pwm.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(&htim2, &pwm, channel) != HAL_OK) {
        Error_Handler();
    }
}

static void TIM2_PWM_Init(void)
{
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7U;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999U;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }

    PWM_Channel_Init(TIM_CHANNEL_1, 100U);
    PWM_Channel_Init(TIM_CHANNEL_2, 300U);
    PWM_Channel_Init(TIM_CHANNEL_3, 500U);
    PWM_Channel_Init(TIM_CHANNEL_4, 700U);

    if ((HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1) != HAL_OK) ||
        (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2) != HAL_OK) ||
        (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3) != HAL_OK) ||
        (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4) != HAL_OK)) {
        Error_Handler();
    }
}

int main(void)
{
    Platform_Init();
    GPIO_Init();
    TIM2_PWM_Init();

    while (1) {
        __WFI();
    }
}
