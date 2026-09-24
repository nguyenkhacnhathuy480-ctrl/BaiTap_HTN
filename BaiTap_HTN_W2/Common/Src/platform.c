#include "platform.h"

static volatile uint32_t system_milliseconds;

__attribute__((weak)) void App_SysTick_1ms(void)
{
}

void SysTick_Handler(void)
{
    ++system_milliseconds;
    App_SysTick_1ms();
}

void Platform_Init(void)
{
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0U) {
    }

    RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE |
                   RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
    RCC->CFGR |= RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) {
    }

    SystemCoreClock = PLATFORM_CLOCK_HZ;
}

void Platform_SysTickStart1ms(void)
{
    system_milliseconds = 0U;
    if (SysTick_Config(PLATFORM_CLOCK_HZ / 1000U) != 0U) {
        Error_Handler();
    }
}

uint32_t Platform_Millis(void)
{
    return system_milliseconds;
}

void Platform_DelayMs(uint32_t milliseconds)
{
    const uint32_t start = Platform_Millis();

    while ((Platform_Millis() - start) < milliseconds) {
        __WFI();
    }
}

void UART1_Init(uint32_t baud_rate)
{
    uint32_t crh;

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    crh = GPIOA->CRH;
    crh &= ~((0xFU << 4U) | (0xFU << 8U));
    crh |= (0xBU << 4U) | (0x4U << 8U);
    GPIOA->CRH = crh;

    USART1->CR1 = 0U;
    USART1->CR2 = 0U;
    USART1->CR3 = 0U;
    USART1->BRR = (PLATFORM_CLOCK_HZ + (baud_rate / 2U)) / baud_rate;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void UART1_WriteByte(uint8_t value)
{
    while ((USART1->SR & USART_SR_TXE) == 0U) {
    }
    USART1->DR = value;
}

void UART1_WriteString(const char *text)
{
    while (*text != '\0') {
        UART1_WriteByte((uint8_t)*text++);
    }
}

void UART1_WriteU32(uint32_t value)
{
    char digits[10];
    uint32_t count = 0U;

    do {
        digits[count++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);

    while (count != 0U) {
        UART1_WriteByte((uint8_t)digits[--count]);
    }
}

uint8_t UART1_ReadByteBlocking(void)
{
    while ((USART1->SR & USART_SR_RXNE) == 0U) {
    }
    return (uint8_t)USART1->DR;
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {
        __WFI();
    }
}
