#include "board.h"

uint32_t SystemCoreClock = 8000000U;
void Board_Write(GPIO_TypeDef *port, uint32_t pin, uint8_t high)
{
    port->BSRR = high ? (1U << pin) : (1U << (pin + 16U));
}
void Board_Output(GPIO_TypeDef *port, uint32_t pin)
{
    volatile uint32_t *cr = pin < 8U ? &port->CRL : &port->CRH;
    uint32_t shift = (pin & 7U) * 4U;
    *cr = (*cr & ~(15U << shift)) | (2U << shift);
}
void Board_EmergencyStop(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
    Board_Write(BOARD_STBY_PORT, BOARD_STBY_PIN, 0U);
    Board_Output(BOARD_STBY_PORT, BOARD_STBY_PIN);
    TIM1->CCR1 = 0U; TIM1->CCR2 = 0U;
}
static void wait_clock(volatile uint32_t *reg, uint32_t mask, uint32_t expected)
{
    uint32_t timeout = 2000000U;
    while ((*reg & mask) != expected) {
        if (--timeout == 0U) { Board_EmergencyStop(); for (;;) {} }
    }
}
/* Called by the Keil startup before __main. Do not rely on initialized globals. */
void SystemInit(void)
{
    RCC->CR |= RCC_CR_HSION;
    RCC->CFGR = 0U;
    RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_HSEON | RCC_CR_CSSON);
    SCB->VTOR = 0x08000000U;
}
void Board_Init(void)
{
    Board_EmergencyStop();
#if BOARD_OLED_TEST_ONLY || BOARD_MOTOR_TEST_ONLY
    /* OLED bring-up must not depend on an external crystal or PLL. */
    RCC->CR |= RCC_CR_HSION;
    wait_clock(&RCC->CR,RCC_CR_HSIRDY,RCC_CR_HSIRDY);
    RCC->CFGR=0U;
    wait_clock(&RCC->CFGR,RCC_CFGR_SWS,RCC_CFGR_SWS_HSI);
    SystemCoreClock=8000000U;
#else
    RCC->CR |= RCC_CR_HSEON;
    wait_clock(&RCC->CR, RCC_CR_HSERDY, RCC_CR_HSERDY);
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;
    RCC->CFGR = RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6;
    RCC->CR |= RCC_CR_PLLON;
    wait_clock(&RCC->CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY);
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    wait_clock(&RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_PLL);
    SystemCoreClock = BOARD_CPU_HZ;
#endif
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    NVIC_SetPriorityGrouping(0U);
    Board_Write(BOARD_LED_PORT, BOARD_LED_PIN, 1U);
    Board_Output(BOARD_LED_PORT, BOARD_LED_PIN);
}
void Board_LedToggle(void) { BOARD_LED_PORT->ODR ^= 1U << BOARD_LED_PIN; }
void Board_DelayUs(uint32_t us)
{
    uint32_t start=DWT->CYCCNT, cycles=us*(SystemCoreClock/1000000U);
    while ((uint32_t)(DWT->CYCCNT-start)<cycles) {}
}
