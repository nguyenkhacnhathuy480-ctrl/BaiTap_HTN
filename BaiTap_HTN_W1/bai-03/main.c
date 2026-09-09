#include <stdint.h>

/* STM32F103 register addresses. */
#define RCC_APB2ENR   (*(volatile uint32_t *)0x40021018u)
#define AFIO_MAPR     (*(volatile uint32_t *)0x40010004u)
#define GPIOA_CRL     (*(volatile uint32_t *)0x40010800u)
#define GPIOA_CRH     (*(volatile uint32_t *)0x40010804u)
#define GPIOA_IDR     (*(volatile uint32_t *)0x40010808u)
#define GPIOA_ODR     (*(volatile uint32_t *)0x4001080Cu)
#define GPIOA_BSRR    (*(volatile uint32_t *)0x40010810u)

#define RCC_AFIOEN    (1u << 0)
#define RCC_IOPAEN    (1u << 2)
#define AFIO_SWJ_NONE (4u << 24)

int main(void)
{
    /* Enable AFIO and GPIOA clocks. */
    RCC_APB2ENR |= RCC_AFIOEN | RCC_IOPAEN;

    /*
     * PA0-PA7: input with pull-up.
     * Each CRL nibble is CNF=10, MODE=00 -> 0b1000 (0x8).
     */
    GPIOA_CRL = 0x88888888u;
    GPIOA_ODR |= 0x00FFu;

    /*
     * PA8-PA15: 2 MHz general-purpose push-pull output.
     * Each CRH nibble is CNF=00, MODE=10 -> 0b0010 (0x2).
     */
    GPIOA_CRH = 0x22222222u;

    /* Release PA13, PA14 and PA15 from SWD/JTAG so all eight pins are GPIO. */
    AFIO_MAPR = AFIO_SWJ_NONE;

    for (;;) {
        uint32_t input = GPIOA_IDR & 0x00FFu;
        uint32_t inverted = (~input) & 0x00FFu;

        /* Reset PA8-PA15, then set the required output bits atomically. */
        GPIOA_BSRR = (0x00FFu << 24) | (inverted << 8);
    }
}
