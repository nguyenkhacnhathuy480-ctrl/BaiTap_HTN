/* ADC1 scan IN0..8, one coherent frame via normal DMA1 channel 1. */
#include "line_sensor.h"
#include "board.h"
static volatile uint16_t adc_dma[9];
static uint8_t wait_clear(volatile uint32_t *reg,uint32_t bits)
{
    uint32_t start=DWT->CYCCNT;
    while ((*reg & bits)!=0U) {
        if ((uint32_t)(DWT->CYCCNT-start)>BOARD_CPU_HZ/100U) return 0U;
    }
    return 1U;
}
uint8_t Sensors_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN|RCC_APB2ENR_IOPBEN|RCC_APB2ENR_ADC1EN;
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    GPIOA->CRL=0U;
    GPIOB->CRL &= ~15U;
    ADC1->CR1=ADC_CR1_SCAN;
    /* ADC 12 MHz, 239.5-cycle sample time for the 10k pull-up/RC network. */
    ADC1->SMPR2=0x07FFFFFFU;
    ADC1->SQR1=8U<<20U;
    ADC1->SQR2=6U|(7U<<5U)|(8U<<10U);
    ADC1->SQR3=(1U<<5U)|(2U<<10U)|(3U<<15U)|(4U<<20U)|(5U<<25U);
    ADC1->CR2=ADC_CR2_ADON;
    Board_DelayUs(10U);
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    if (!wait_clear(&ADC1->CR2,ADC_CR2_RSTCAL)) return 0U;
    ADC1->CR2 |= ADC_CR2_CAL;
    return wait_clear(&ADC1->CR2,ADC_CR2_CAL);
}
uint8_t Sensors_Read(uint16_t values[9])
{
    uint32_t start,i;
    DMA1_Channel1->CCR=0U;
    DMA1->IFCR=15U;
    DMA1_Channel1->CPAR=(uint32_t)&ADC1->DR;
    DMA1_Channel1->CMAR=(uint32_t)adc_dma;
    DMA1_Channel1->CNDTR=9U;
    DMA1_Channel1->CCR=DMA_CCR_MINC|DMA_CCR_PSIZE_0|DMA_CCR_MSIZE_0|DMA_CCR_PL_1|DMA_CCR_EN;
    ADC1->SR=0U;
    ADC1->CR2=ADC_CR2_ADON|ADC_CR2_DMA|ADC_CR2_EXTTRIG|ADC_CR2_EXTSEL;
    ADC1->CR2 |= ADC_CR2_SWSTART;
    start=DWT->CYCCNT;
    while ((DMA1->ISR & DMA_ISR_TCIF1)==0U) {
        if ((DMA1->ISR & DMA_ISR_TEIF1) || (uint32_t)(DWT->CYCCNT-start)>BOARD_CPU_HZ/1000U) {
            DMA1_Channel1->CCR=0U; ADC1->CR2=0U; return 0U;
        }
    }
    DMA1_Channel1->CCR=0U;
    __DMB();
    for (i=0U;i<9U;++i) values[i]=adc_dma[i];
    return 1U;
}
