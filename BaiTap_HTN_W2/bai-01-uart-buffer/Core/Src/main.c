#include "stm32f1xx.h"

#include <stdint.h>

#define SYSTEM_CLOCK_HZ 8000000U
#define RX_BUFFER_SIZE  64U

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

static void UART1_Init(uint32_t baud_rate)
{
    uint32_t crh;

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    crh = GPIOA->CRH;
    crh &= ~((0xFU << 4U) | (0xFU << 8U));
    crh |= (0xBU << 4U) | (0x4U << 8U); /* PA9 TX, PA10 RX. */
    GPIOA->CRH = crh;

    USART1->CR1 = 0U;
    USART1->CR2 = 0U;
    USART1->CR3 = 0U;
    USART1->BRR = (SYSTEM_CLOCK_HZ + (baud_rate / 2U)) / baud_rate;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

static void UART1_WriteByte(uint8_t value)
{
    while ((USART1->SR & USART_SR_TXE) == 0U) {
    }
    USART1->DR = value;
}

static void UART1_WriteString(const char *text)
{
    while (*text != '\0') {
        UART1_WriteByte((uint8_t)*text++);
    }
}

static uint8_t UART1_ReadByte(void)
{
    while ((USART1->SR & USART_SR_RXNE) == 0U) {
    }
    return (uint8_t)USART1->DR;
}

int main(void)
{
    static const char prefix[] = "ELE141502: ";
    char buffer[RX_BUFFER_SIZE];
    uint16_t length = 0U;

    Clock_Init();
    UART1_Init(115200U);
    UART1_WriteString("Nhap chuoi, ket thuc bang dau !\r\n");

    while (1) {
        const uint8_t received = UART1_ReadByte();

        if (received == (uint8_t)'!') {
            buffer[length] = '\0';
            UART1_WriteString(prefix);
            UART1_WriteString(buffer);
            UART1_WriteString("\n\r");
            length = 0U;
        } else if ((received == (uint8_t)'\r') ||
                   (received == (uint8_t)'\n')) {
            /* Dau ket thuc ban tin la '!', bo qua xuong dong cua terminal. */
        } else if (length < (RX_BUFFER_SIZE - 1U)) {
            buffer[length++] = (char)received;
        } else {
            length = 0U;
            UART1_WriteString("ERROR: BUFFER FULL\n\r");
        }
    }
}
