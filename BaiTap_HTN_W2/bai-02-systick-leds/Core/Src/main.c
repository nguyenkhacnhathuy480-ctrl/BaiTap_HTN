#include "stm32f1xx.h"

#include <stdint.h>

#define SYSTEM_CLOCK_HZ 8000000U

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

static void LED_GPIO_Init(void)
{
    uint32_t crl;

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    crl = GPIOA->CRL;
    crl &= ~0xFFFU;
    crl |= 0x222U; /* PA0..PA2: output push-pull 2 MHz. */
    GPIOA->CRL = crl;
    GPIOA->BRR = GPIO_BRR_BR0 | GPIO_BRR_BR1 | GPIO_BRR_BR2;
}

void SysTick_Handler(void)
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
    Clock_Init();
    LED_GPIO_Init();

    if (SysTick_Config(SYSTEM_CLOCK_HZ / 1000U) != 0U) {
        while (1) {
        }
    }

    while (1) {
        __WFI();
    }
}
