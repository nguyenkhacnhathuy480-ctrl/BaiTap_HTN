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

static void TIM2_PWM_Init(void)
{
    uint32_t crl;

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    crl = GPIOA->CRL;
    crl &= ~0xFFFFU;
    crl |= 0xBBBBU; /* PA0..PA3: alternate-function push-pull. */
    GPIOA->CRL = crl;

    TIM2->PSC = 7U;
    TIM2->ARR = 999U; /* 8 MHz / (7 + 1) / (999 + 1) = 1 kHz. */
    TIM2->CCR1 = 100U;
    TIM2->CCR2 = 300U;
    TIM2->CCR3 = 500U;
    TIM2->CCR4 = 700U;

    TIM2->CCMR1 = TIM_CCMR1_OC1PE | (6U << TIM_CCMR1_OC1M_Pos) |
                   TIM_CCMR1_OC2PE | (6U << TIM_CCMR1_OC2M_Pos);
    TIM2->CCMR2 = TIM_CCMR2_OC3PE | (6U << TIM_CCMR2_OC3M_Pos) |
                   TIM_CCMR2_OC4PE | (6U << TIM_CCMR2_OC4M_Pos);
    TIM2->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E |
                 TIM_CCER_CC3E | TIM_CCER_CC4E;
    TIM2->CR1 = TIM_CR1_ARPE;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;
}

int main(void)
{
    Clock_Init();
    TIM2_PWM_Init();

    while (1) {
        __WFI();
    }
}
