#include "platform.h"

#include <stdint.h>

static void LED_GPIO_Init(void)
{
    uint32_t crl;

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    crl = GPIOA->CRL;
    crl &= ~0xFFFU;
    crl |= 0x222U;
    GPIOA->CRL = crl;
    GPIOA->BRR = GPIO_BRR_BR0 | GPIO_BRR_BR1 | GPIO_BRR_BR2;
}

void App_SysTick_1ms(void)
{
    static uint32_t led_01hz_ms;
    static uint32_t led_1hz_ms;
    static uint32_t led_10hz_ms;

    if (++led_01hz_ms >= 5000U) {
        led_01hz_ms = 0U;
        GPIOA->ODR ^= GPIO_ODR_ODR0;
    }

    if (++led_1hz_ms >= 500U) {
        led_1hz_ms = 0U;
        GPIOA->ODR ^= GPIO_ODR_ODR1;
    }

    if (++led_10hz_ms >= 50U) {
        led_10hz_ms = 0U;
        GPIOA->ODR ^= GPIO_ODR_ODR2;
    }
}

int main(void)
{
    Platform_Init();
    LED_GPIO_Init();
    Platform_SysTickStart1ms();

    while (1) {
        __WFI();
    }
}
