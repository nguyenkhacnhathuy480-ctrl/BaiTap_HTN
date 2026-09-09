#include <stdint.h>

/* STM32F103 register addresses. */
#define RCC_APB2ENR   (*(volatile uint32_t *)0x40021018u)
#define GPIOA_CRL     (*(volatile uint32_t *)0x40010800u)
#define GPIOA_IDR     (*(volatile uint32_t *)0x40010808u)
#define GPIOA_ODR     (*(volatile uint32_t *)0x4001080Cu)
#define GPIOC_CRH     (*(volatile uint32_t *)0x40011004u)
#define GPIOC_BSRR    (*(volatile uint32_t *)0x40011010u)

#define SYST_CSR      (*(volatile uint32_t *)0xE000E010u)
#define SYST_RVR      (*(volatile uint32_t *)0xE000E014u)
#define SYST_CVR      (*(volatile uint32_t *)0xE000E018u)

#define RCC_IOPAEN    (1u << 2)
#define RCC_IOPCEN    (1u << 4)
#define BUTTON_PIN    0u
#define LED_PIN       13u

static void systick_init(void)
{
    /* Reset clock is HSI = 8 MHz. One COUNTFLAG event is 1 ms. */
    SYST_RVR = 8000u - 1u;
    SYST_CVR = 0u;
    SYST_CSR = (1u << 2) | (1u << 0); /* CPU clock, counter enabled. */
}

static void delay_ms(uint32_t milliseconds)
{
    while (milliseconds-- != 0u) {
        while ((SYST_CSR & (1u << 16)) == 0u) {
            /* Wait for COUNTFLAG. */
        }
    }
}
static uint32_t button_is_pressed(void)
{
    /* PA0 uses pull-up, so a press to GND reads as zero. */
    return ((GPIOA_IDR & (1u << BUTTON_PIN)) == 0u) ? 1u : 0u;
}

static void led_write(uint32_t on)
{
    /* The Blue Pill onboard PC13 LED is active low. */
    if (on != 0u) {
        GPIOC_BSRR = 1u << (LED_PIN + 16u);
    } else {
        GPIOC_BSRR = 1u << LED_PIN;
    }
}

int main(void)
{
    RCC_APB2ENR |= RCC_IOPAEN | RCC_IOPCEN;

    /* PA0: input with pull-up (CNF=10, MODE=00). */
    GPIOA_CRL = (GPIOA_CRL & ~0xFu) | 0x8u;
    GPIOA_ODR |= 1u << BUTTON_PIN;

    /* PC13: 2 MHz general-purpose push-pull output (CNF=00, MODE=10). */
    GPIOC_CRH = (GPIOC_CRH & ~(0xFu << 20)) | (0x2u << 20);

    uint32_t led_on = 0u;
    uint32_t stable_pressed = button_is_pressed();
    uint32_t press_seen = stable_pressed;
    led_write(led_on);
    systick_init();

    for (;;) {
        uint32_t sample = button_is_pressed();

        if (sample != stable_pressed) {
            delay_ms(20u);
            sample = button_is_pressed();

            if (sample != stable_pressed) {
                stable_pressed = sample;

                if (stable_pressed != 0u) {
                    press_seen = 1u;
                } else if (press_seen != 0u) {
                    /* Toggle only after a complete press-and-release action. */
                    led_on ^= 1u;
                    led_write(led_on);
                    press_seen = 0u;
                }
            }
        }
    }
}
