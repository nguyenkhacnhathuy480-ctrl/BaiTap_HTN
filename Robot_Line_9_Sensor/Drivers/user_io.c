#include "user_io.h"
#include "board.h"
void UserIO_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    GPIOB->CRL=(GPIOB->CRL & ~(15U<<20U))|(8U<<20U);
    GPIOB->CRH=(GPIOB->CRH & ~255U)|0x88U;
    GPIOB->BSRR=(1U<<5U)|(1U<<8U)|(1U<<9U);
    Board_Write(GPIOB,1U,0U);
#if BOARD_BUZZER_PASSIVE
    GPIOB->CRL=(GPIOB->CRL & ~(15U<<4U))|(10U<<4U);
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    /* APB1 timer clock equals core clock in both board clock profiles. */
    TIM3->PSC=SystemCoreClock/1000000U-1U; TIM3->ARR=499U; TIM3->CCR4=0U;
    TIM3->CCMR2=(6U<<12U)|TIM_CCMR2_OC4PE;
    TIM3->CCER=TIM_CCER_CC4E;
    TIM3->EGR=TIM_EGR_UG; TIM3->CR1=TIM_CR1_CEN;
#else
    Board_Output(GPIOB,1U);
#endif
}
uint8_t Buttons_Read(void)
{
    uint32_t input=~GPIOB->IDR;
    return (uint8_t)(((input>>5U)&1U)|(((input>>8U)&1U)<<1U)|(((input>>9U)&1U)<<2U));
}
void Buzzer_Set(uint8_t on)
{
#if BOARD_BUZZER_PASSIVE
    TIM3->CCR4=on?250U:0U;
#else
    Board_Write(GPIOB,1U,on);
#endif
}
