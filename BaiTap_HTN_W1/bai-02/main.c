#include <stdint.h>

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
    /* Clock mac dinh HSI = 8 MHz, 8000 tick tuong ung 1 ms. */
    SYST_RVR = 8000u - 1u;
    SYST_CVR = 0u;
    SYST_CSR = (1u << 2) | (1u << 0);
}

static void delay_ms(uint32_t milliseconds)
{
    while (milliseconds-- != 0u) {
        while ((SYST_CSR & (1u << 16)) == 0u) {
            /* Cho co COUNTFLAG. */
        }
    }
}

static void show_one_led(uint32_t index)
{
    uint32_t selected = 1u << index;
    uint32_t leds_to_reset = LED_MASK & ~selected;

    GPIOA_BSRR = (leds_to_reset << 16) | selected;
}

int main(void)
{
    RCC_APB2ENR |= RCC_IOPAEN;

    /* PA0-PA7: output push-pull 2 MHz. */
    GPIOA_CRL = 0x22222222u;
    systick_init();

    for (;;) {
        /* Chay tu PA0 den PA7. */
        for (uint32_t i = 0u; i < 8u; ++i) {
            show_one_led(i);
            delay_ms(LED_STEP_MS);
        }

        /* Chay nguoc tu PA6 ve PA0, khong lap lai PA7. */
        for (int32_t i = 6; i >= 0; --i) {
            show_one_led((uint32_t)i);
            delay_ms(LED_STEP_MS);
        }
    }
}
