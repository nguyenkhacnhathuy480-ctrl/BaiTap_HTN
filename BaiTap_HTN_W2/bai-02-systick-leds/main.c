#include <stdint.h>

#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018u)
#define GPIOA_CRL   (*(volatile uint32_t *)0x40010800u)
#define GPIOA_BSRR  (*(volatile uint32_t *)0x40010810u)

#define SYST_CSR    (*(volatile uint32_t *)0xE000E010u)
#define SYST_RVR    (*(volatile uint32_t *)0xE000E014u)
#define SYST_CVR    (*(volatile uint32_t *)0xE000E018u)

#define RCC_IOPAEN  (1u << 2)

static volatile uint32_t led_state;

static void led_toggle(uint32_t pin)
{
    uint32_t mask = 1u << pin;
    led_state ^= mask;

    if ((led_state & mask) != 0u) {
        GPIOA_BSRR = mask;
    } else {
        GPIOA_BSRR = mask << 16;
    }
}

void SysTick_Handler(void)
{
    static uint32_t counter_01hz;
    static uint32_t counter_1hz;
    static uint32_t counter_10hz;

    if (++counter_10hz >= 50u) {
        counter_10hz = 0u;
        led_toggle(2u);
    }

    if (++counter_1hz >= 500u) {
        counter_1hz = 0u;
        led_toggle(1u);
    }

    if (++counter_01hz >= 5000u) {
        counter_01hz = 0u;
        led_toggle(0u);
    }
}

static void systick_init(void)
{
    /* HSI 8 MHz: 8000 clock cycles produce a 1 ms interrupt. */
    SYST_RVR = 8000u - 1u;
    SYST_CVR = 0u;
    SYST_CSR = (1u << 2) | (1u << 1) | (1u << 0);
}

int main(void)
{
    RCC_APB2ENR |= RCC_IOPAEN;

    /* PA0, PA1, PA2: general-purpose push-pull outputs at 2 MHz. */
    GPIOA_CRL = (GPIOA_CRL & ~0xFFFu) | 0x222u;
    GPIOA_BSRR = (0x7u << 16);
    led_state = 0u;

    systick_init();

    for (;;) {
        __asm volatile ("wfi");
    }
}
