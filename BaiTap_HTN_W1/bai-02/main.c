#include <stdint.h>

/* STM32F103 register addresses. */
#define RCC_APB2ENR   (*(volatile uint32_t *)0x40021018u)
#define GPIOA_CRL     (*(volatile uint32_t *)0x40010800u)
#define GPIOA_BSRR    (*(volatile uint32_t *)0x40010810u)

#define SYST_CSR      (*(volatile uint32_t *)0xE000E010u)
#define SYST_RVR      (*(volatile uint32_t *)0xE000E014u)
#define SYST_CVR      (*(volatile uint32_t *)0xE000E018u)

#define RCC_IOPAEN    (1u << 2)
#define LED_MASK      0xFFu

#ifndef LED_STEP_MS
#define LED_STEP_MS 150u
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

static void show_one_led(uint32_t index)
{
    uint32_t selected = 1u << index;
    uint32_t leds_to_reset = LED_MASK & ~selected;

    /* Set the selected output and reset the other seven in one write. */
    GPIOA_BSRR = (leds_to_reset << 16) | selected;
}

int main(void)
{
    RCC_APB2ENR |= RCC_IOPAEN;

    /* PA0-PA7: 2 MHz general-purpose push-pull outputs. */
    GPIOA_CRL = 0x22222222u;

    uint32_t led_index = 0u;
    int32_t direction = 1;
    systick_init();

    for (;;) {
        show_one_led(led_index);
        delay_ms(LED_STEP_MS);

        if (led_index == 7u) {
            direction = -1;
        } else if (led_index == 0u) {
            direction = 1;
        }

        led_index = (uint32_t)((int32_t)led_index + direction);
    }
}
