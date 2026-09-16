#include <stdint.h>

#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018u)
#define GPIOA_CRH   (*(volatile uint32_t *)0x40010804u)

#define USART1_SR   (*(volatile uint32_t *)0x40013800u)
#define USART1_DR   (*(volatile uint32_t *)0x40013804u)
#define USART1_BRR  (*(volatile uint32_t *)0x40013808u)
#define USART1_CR1  (*(volatile uint32_t *)0x4001380Cu)

#define RCC_AFIOEN   (1u << 0)
#define RCC_IOPAEN   (1u << 2)
#define RCC_USART1EN (1u << 14)

#define USART_SR_RXNE (1u << 5)
#define USART_SR_TXE  (1u << 7)
#define USART_CR1_RE  (1u << 2)
#define USART_CR1_TE  (1u << 3)
#define USART_CR1_UE  (1u << 13)

#define CLASS_CODE "02"
#define GROUP_CODE "01"
#define RX_BUFFER_SIZE 128u

static void uart_init(void)
{
    RCC_APB2ENR |= RCC_AFIOEN | RCC_IOPAEN | RCC_USART1EN;

    /* PA9: alternate-function push-pull 50 MHz; PA10: floating input. */
    GPIOA_CRH = (GPIOA_CRH & ~((0xFu << 4) | (0xFu << 8))) |
                (0xBu << 4) | (0x4u << 8);

    /* PCLK2 = 8 MHz, USARTDIV = 8 MHz / 115200, BRR = 0x45. */
    USART1_BRR = 0x45u;
    USART1_CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

static char uart_read_char(void)
{
    while ((USART1_SR & USART_SR_RXNE) == 0u) {
    }
    return (char)(USART1_DR & 0xFFu);
}

static void uart_write_char(char value)
{
    while ((USART1_SR & USART_SR_TXE) == 0u) {
    }
    USART1_DR = (uint32_t)(uint8_t)value;
}

static void uart_write_string(const char *text)
{
    while (*text != '\0') {
        uart_write_char(*text++);
    }
}

int main(void)
{
    char receive_buffer[RX_BUFFER_SIZE];
    uint32_t length = 0u;

    uart_init();
    uart_write_string("UART1 ready. End each message with !\r\n");

    for (;;) {
        char received = uart_read_char();

        if (received == '!') {
            receive_buffer[length] = '\0';
            uart_write_string(CLASS_CODE GROUP_CODE ": ");
            uart_write_string(receive_buffer);
            uart_write_string("\n\r");
            length = 0u;
        } else if (length < (RX_BUFFER_SIZE - 1u)) {
            receive_buffer[length++] = received;
        }
    }
}
