#include <stdint.h>

#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018u)
#define RCC_APB1ENR (*(volatile uint32_t *)0x4002101Cu)
#define GPIOA_CRL   (*(volatile uint32_t *)0x40010800u)
#define GPIOA_CRH   (*(volatile uint32_t *)0x40010804u)

#define TIM2_CR1    (*(volatile uint32_t *)0x40000000u)
#define TIM2_EGR    (*(volatile uint32_t *)0x40000014u)
#define TIM2_CCMR1  (*(volatile uint32_t *)0x40000018u)
#define TIM2_CCER   (*(volatile uint32_t *)0x40000020u)
#define TIM2_PSC    (*(volatile uint32_t *)0x40000028u)
#define TIM2_ARR    (*(volatile uint32_t *)0x4000002Cu)
#define TIM2_CCR1   (*(volatile uint32_t *)0x40000034u)

#define USART1_SR   (*(volatile uint32_t *)0x40013800u)
#define USART1_DR   (*(volatile uint32_t *)0x40013804u)
#define USART1_BRR  (*(volatile uint32_t *)0x40013808u)
#define USART1_CR1  (*(volatile uint32_t *)0x4001380Cu)

#define NVIC_ISER1  (*(volatile uint32_t *)0xE000E104u)

#define COMMAND_BUFFER_SIZE 32u

static volatile char command_buffer[COMMAND_BUFFER_SIZE];
static volatile uint32_t command_length;
static volatile uint32_t command_ready;

static uint32_t led_enabled;
static uint32_t pwm_percent = 50u;

static void uart_write_char(char value)
{
    while ((USART1_SR & (1u << 7)) == 0u) {
    }
    USART1_DR = (uint32_t)(uint8_t)value;
}

static void uart_write_string(const char *text)
{
    while (*text != '\0') {
        uart_write_char(*text++);
    }
}

static void uart_write_uint(uint32_t value)
{
    char digits[10];
    uint32_t count = 0u;

    if (value == 0u) {
        uart_write_char('0');
        return;
    }

    while (value != 0u) {
        digits[count++] = (char)('0' + (value % 10u));
        value /= 10u;
    }

    while (count != 0u) {
        uart_write_char(digits[--count]);
    }
}

static char to_upper(char value)
{
    if ((value >= 'a') && (value <= 'z')) {
        value = (char)(value - ('a' - 'A'));
    }
    return value;
}

static uint32_t string_equals_ignore_case(const char *left, const char *right)
{
    while ((*left != '\0') && (*right != '\0')) {
        if (to_upper(*left++) != to_upper(*right++)) {
            return 0u;
        }
    }
    return ((*left == '\0') && (*right == '\0')) ? 1u : 0u;
}

static uint32_t starts_with_pwm(const char *text)
{
    return (to_upper(text[0]) == 'P') &&
           (to_upper(text[1]) == 'W') &&
           (to_upper(text[2]) == 'M') &&
           (text[3] == ':');
}

static uint32_t parse_pwm_percent(const char *text, uint32_t *result)
{
    uint32_t index = 4u;
    uint32_t value = 0u;
    uint32_t has_digit = 0u;

    while ((text[index] >= '0') && (text[index] <= '9')) {
        has_digit = 1u;
        value = (value * 10u) + (uint32_t)(text[index] - '0');
        ++index;
    }

    if (text[index] == '%') {
        ++index;
    }

    if ((has_digit == 0u) || (text[index] != '\0') || (value > 100u)) {
        return 0u;
    }

    *result = value;
    return 1u;
}

static void apply_pwm(void)
{
    TIM2_CCR1 = (led_enabled != 0u) ? (pwm_percent * 10u) : 0u;
}

static void send_status(void)
{
    uart_write_string("STATUS:");
    uart_write_string((led_enabled != 0u) ? "ON" : "OFF");
    uart_write_string(" PWM:");
    uart_write_uint(pwm_percent);
    uart_write_string("%\r\n");
}

static void process_command(const char *command)
{
    uint32_t new_percent;

    if (string_equals_ignore_case(command, "ON") != 0u) {
        led_enabled = 1u;
        apply_pwm();
        uart_write_string("OK ON\r\n");
    } else if (string_equals_ignore_case(command, "OFF") != 0u) {
        led_enabled = 0u;
        apply_pwm();
        uart_write_string("OK OFF\r\n");
    } else if (string_equals_ignore_case(command, "STATUS") != 0u) {
        send_status();
    } else if ((starts_with_pwm(command) != 0u) &&
               (parse_pwm_percent(command, &new_percent) != 0u)) {
        pwm_percent = new_percent;
        apply_pwm();
        uart_write_string("OK PWM:");
        uart_write_uint(pwm_percent);
        uart_write_string("%\r\n");
    } else {
        uart_write_string("ERROR\r\n");
    }
}

void USART1_IRQHandler(void)
{
    uint32_t status = USART1_SR;

    if ((status & (1u << 5)) != 0u) {
        char received = (char)(USART1_DR & 0xFFu);

        if (command_ready == 0u) {
            if (received == '!') {
                command_buffer[command_length] = '\0';
                command_length = 0u;
                command_ready = 1u;
            } else if (command_length < (COMMAND_BUFFER_SIZE - 1u)) {
                command_buffer[command_length++] = received;
            }
        }
    } else if ((status & (1u << 3)) != 0u) {
        (void)USART1_DR;
    }
}

static void hardware_init(void)
{
    RCC_APB2ENR |= (1u << 0) | (1u << 2) | (1u << 14);
    RCC_APB1ENR |= (1u << 0);

    /* PA0 TIM2_CH1; PA9 USART1_TX; PA10 USART1_RX. */
    GPIOA_CRL = (GPIOA_CRL & ~0xFu) | 0xBu;
    GPIOA_CRH = (GPIOA_CRH & ~((0xFu << 4) | (0xFu << 8))) |
                (0xBu << 4) | (0x4u << 8);

    TIM2_PSC = 7u;
    TIM2_ARR = 999u;
    TIM2_CCR1 = 0u;
    TIM2_CCMR1 = (6u << 4) | (1u << 3);
    TIM2_CCER = 1u;
    TIM2_EGR = 1u;
    TIM2_CR1 = (1u << 7) | (1u << 0);

    USART1_BRR = 0x45u;
    USART1_CR1 = (1u << 13) | (1u << 5) | (1u << 3) | (1u << 2);

    /* USART1 is IRQ37, therefore bit 5 in NVIC ISER1. */
    NVIC_ISER1 = (1u << 5);
}

int main(void)
{
    hardware_init();
    uart_write_string("Commands: ON! OFF! PWM:0..100%! STATUS!\r\n");

    for (;;) {
        if (command_ready != 0u) {
            process_command((const char *)command_buffer);
            command_ready = 0u;
        }
    }
}
