/* Single producer (UI task), IRQ consumer. No printf/RTOS calls in IRQ. */
#include "uart_debug.h"
#include "board.h"
#include <string.h>
#define TX_SIZE 1024U
#define RX_SIZE 32U
static char tx[TX_SIZE],rx[RX_SIZE];
static volatile uint16_t tx_head,tx_tail,rx_head,rx_tail;
volatile uint32_t uart_tx_dropped,uart_rx_dropped;
void Uart_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    GPIOB->CRH=(GPIOB->CRH & ~(255U<<8U))|(0x4BU<<8U);
    /* Diagnostic target: HSI 8 MHz, APB1 /1. Robot: 72 MHz, APB1 /2. */
#if BOARD_OLED_TEST_ONLY || BOARD_MOTOR_TEST_ONLY
    USART3->BRR=(SystemCoreClock+57600U)/115200U;
#else
    USART3->BRR=(SystemCoreClock/2U+57600U)/115200U;
#endif
    USART3->CR1=USART_CR1_UE|USART_CR1_TE|USART_CR1_RE|USART_CR1_RXNEIE;
    NVIC_SetPriority(USART3_IRQn,6U); NVIC_EnableIRQ(USART3_IRQn);
}
uint8_t Uart_Write(const char *text)
{
    uint32_t count=(uint32_t)strlen(text),i;
    uint16_t head=tx_head;
    uint32_t free_space=(tx_tail+TX_SIZE-head-1U)%TX_SIZE;
    if (count>free_space) { ++uart_tx_dropped; return 0U; }
    for (i=0U;i<count;++i) { tx[head]=text[i]; head=(uint16_t)((head+1U)%TX_SIZE); }
    __DMB(); tx_head=head;
    USART3->CR1 |= USART_CR1_TXEIE;
    return 1U;
}
int Uart_Read(void)
{
    int value;
    if (rx_head==rx_tail) return -1;
    value=(unsigned char)rx[rx_tail];
    rx_tail=(uint16_t)((rx_tail+1U)%RX_SIZE);
    return value;
}
void USART3_IRQHandler(void)
{
    uint32_t status=USART3->SR;
    if (status & (USART_SR_RXNE|USART_SR_ORE|USART_SR_FE|USART_SR_NE|USART_SR_PE)) {
        char c=(char)USART3->DR; /* SR then DR clears errors */
        if (!(status & (USART_SR_ORE|USART_SR_FE|USART_SR_NE|USART_SR_PE))) {
            uint16_t next=(uint16_t)((rx_head+1U)%RX_SIZE);
            if (next!=rx_tail) { rx[rx_head]=c; __DMB(); rx_head=next; }
            else ++uart_rx_dropped;
        } else ++uart_rx_dropped;
    }
    if ((status & USART_SR_TXE) && (USART3->CR1 & USART_CR1_TXEIE)) {
        if (tx_tail==tx_head) USART3->CR1 &= ~USART_CR1_TXEIE;
        else { USART3->DR=(uint8_t)tx[tx_tail]; tx_tail=(uint16_t)((tx_tail+1U)%TX_SIZE); }
    }
}
