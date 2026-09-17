#include "platform.h"

#include <stdint.h>

static void GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);
    HAL_GPIO_WritePin(GPIOA, gpio.Pin, GPIO_PIN_RESET);
}

void App_SysTick_1ms(void)
{
    static uint32_t led_01hz_ms;
    static uint32_t led_1hz_ms;
    static uint32_t led_10hz_ms;

    if (++led_01hz_ms >= 5000U) {
        led_01hz_ms = 0U;
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
    }

    if (++led_1hz_ms >= 500U) {
        led_1hz_ms = 0U;
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
    }

    if (++led_10hz_ms >= 50U) {
        led_10hz_ms = 0U;
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
    }
}

int main(void)
{
    Platform_Init();
    GPIO_Init();

    while (1) {
        __WFI();
    }
}
