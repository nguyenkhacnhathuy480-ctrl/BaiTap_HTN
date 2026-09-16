#include <stdint.h>

#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018u)
#define RCC_APB1ENR (*(volatile uint32_t *)0x4002101Cu)
#define GPIOA_CRL   (*(volatile uint32_t *)0x40010800u)

#define TIM2_CR1    (*(volatile uint32_t *)0x40000000u)
#define TIM2_EGR    (*(volatile uint32_t *)0x40000014u)
#define TIM2_CCMR1  (*(volatile uint32_t *)0x40000018u)
#define TIM2_CCMR2  (*(volatile uint32_t *)0x4000001Cu)
#define TIM2_CCER   (*(volatile uint32_t *)0x40000020u)
#define TIM2_PSC    (*(volatile uint32_t *)0x40000028u)
#define TIM2_ARR    (*(volatile uint32_t *)0x4000002Cu)
#define TIM2_CCR1   (*(volatile uint32_t *)0x40000034u)
#define TIM2_CCR2   (*(volatile uint32_t *)0x40000038u)
#define TIM2_CCR3   (*(volatile uint32_t *)0x4000003Cu)
#define TIM2_CCR4   (*(volatile uint32_t *)0x40000040u)

static void pwm_init(void)
{
    RCC_APB2ENR |= (1u << 0) | (1u << 2);
    RCC_APB1ENR |= (1u << 0);

    /* PA0-PA3: alternate-function push-pull outputs at 50 MHz. */
    GPIOA_CRL = (GPIOA_CRL & ~0xFFFFu) | 0xBBBBu;

    /* 8 MHz / (7 + 1) / (999 + 1) = 1 kHz. */
    TIM2_CR1 = 0u;
    TIM2_PSC = 7u;
    TIM2_ARR = 999u;

    TIM2_CCR1 = 100u;
    TIM2_CCR2 = 300u;
    TIM2_CCR3 = 500u;
    TIM2_CCR4 = 700u;

    /* PWM mode 1 and preload enabled for channels 1-4. */
    TIM2_CCMR1 = 0x6868u;
    TIM2_CCMR2 = 0x6868u;
    TIM2_CCER = 0x1111u;

    TIM2_EGR = 1u;
    TIM2_CR1 = (1u << 7) | (1u << 0);
}

int main(void)
{
    pwm_init();

    for (;;) {
        __asm volatile ("wfi");
    }
}
