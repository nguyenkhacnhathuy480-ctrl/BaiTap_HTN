#include <stdint.h>

/* STM32F103 register addresses. */
#define RCC_APB2ENR   (*(volatile uint32_t *)0x40021018u)
#define GPIOC_CRH     (*(volatile uint32_t *)0x40011004u)
#define GPIOC_BSRR    (*(volatile uint32_t *)0x40011010u)

#define SYST_CSR      (*(volatile uint32_t *)0xE000E010u)
#define SYST_RVR      (*(volatile uint32_t *)0xE000E014u)
#define SYST_CVR      (*(volatile uint32_t *)0xE000E018u)

#define RCC_IOPCEN    (1u << 4)
#define LED_PIN       13u

#ifndef BLINK_INTERVAL_MS
#define BLINK_INTERVAL_MS 1000u
#endif

static void systick_init(void)
{
    /* The reset clock is HSI = 8 MHz, so 8000 ticks equal 1 ms. */
    SYST_RVR = 8000u - 1u;
    SYST_CVR = 0u;
    SYST_CSR = (1u << 2) | (1u << 0);
}

static void delay_ms(uint32_t milliseconds)
{
    while (milliseconds-- != 0u) {
        while ((SYST_CSR & (1u << 16)) == 0u) {
            /* Wait for the SysTick COUNTFLAG. */
        }
    }
}

static void led_write(uint32_t on)
{
    /* The onboard PC13 LED is active low. */
    if (on != 0u) {
        GPIOC_BSRR = 1u << (LED_PIN + 16u);
    } else {
        GPIOC_BSRR = 1u << LED_PIN;
    }
}

int main(void)
{
    RCC_APB2ENR |= RCC_IOPCEN;

    /* PC13: 2 MHz general-purpose push-pull output (CNF=00, MODE=10). */
    GPIOC_CRH = (GPIOC_CRH & ~(0xFu << 20)) | (0x2u << 20);

    uint32_t led_on = 0u;
    led_write(led_on);
    systick_init();

    for (;;) {
        delay_ms(BLINK_INTERVAL_MS);
        led_on ^= 1u;
        led_write(led_on);
    }
}
