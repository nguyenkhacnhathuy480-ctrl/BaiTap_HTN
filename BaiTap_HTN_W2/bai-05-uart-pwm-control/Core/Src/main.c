#include "stm32f1xx.h"

#include <stdint.h>

#define SYSTEM_CLOCK_HZ    8000000U
#define COMMAND_BUFFER_SIZE 32U

static volatile char command_buffer[COMMAND_BUFFER_SIZE];
static volatile uint32_t command_length;
static volatile uint32_t command_ready;
static uint32_t led_enabled;
static uint32_t pwm_percent = 50U;

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
    crh |= (0xBU << 4U) | (0x4U << 8U);
    GPIOA->CRH = crh;

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

static void UART1_WriteU32(uint32_t value)
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

static void TIM2_PWM_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    GPIOA->CRL = (GPIOA->CRL & ~0xFU) | 0xBU;

    TIM2->PSC = 7U;
    TIM2->ARR = 999U;
    TIM2->CCR1 = 0U;
    TIM2->CCMR1 = TIM_CCMR1_OC1PE | (6U << TIM_CCMR1_OC1M_Pos);
    TIM2->CCER = TIM_CCER_CC1E;
    TIM2->CR1 = TIM_CR1_ARPE;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;
}

static char ToUpper(char value)
{
    if ((value >= 'a') && (value <= 'z')) {
        value = (char)(value - ('a' - 'A'));
    }
    return value;
}

static uint32_t StringEqualsIgnoreCase(const char *left, const char *right)
{
    while ((*left != '\0') && (*right != '\0')) {
        if (ToUpper(*left++) != ToUpper(*right++)) {
            return 0U;
        }
    }
    return ((*left == '\0') && (*right == '\0')) ? 1U : 0U;
}

static uint32_t ParsePWM(const char *text, uint32_t *result)
{
    uint32_t index = 4U;
    uint32_t value = 0U;
    uint32_t has_digit = 0U;

    if ((ToUpper(text[0]) != 'P') || (ToUpper(text[1]) != 'W') ||
        (ToUpper(text[2]) != 'M') || (text[3] != ':')) {
        return 0U;
    }
    while ((text[index] >= '0') && (text[index] <= '9')) {
        has_digit = 1U;
        value = (value * 10U) + (uint32_t)(text[index] - '0');
        ++index;
    }
    if (text[index] == '%') {
        ++index;
    }
    if ((has_digit == 0U) || (text[index] != '\0') || (value > 100U)) {
        return 0U;
    }
    *result = value;
    return 1U;
}

static void ApplyPWM(void)
{
    TIM2->CCR1 = (led_enabled != 0U) ? (pwm_percent * 10U) : 0U;
}

static void SendStatus(void)
{
    UART1_WriteString("STATUS:");
    UART1_WriteString((led_enabled != 0U) ? "ON" : "OFF");
    UART1_WriteString(" PWM:");
    UART1_WriteU32(pwm_percent);
    UART1_WriteString("%\r\n");
}

static void ProcessCommand(const char *command)
{
    uint32_t new_percent;

    if (StringEqualsIgnoreCase(command, "ON") != 0U) {
        led_enabled = 1U;
        ApplyPWM();
        UART1_WriteString("OK ON\r\n");
    } else if (StringEqualsIgnoreCase(command, "OFF") != 0U) {
        led_enabled = 0U;
        ApplyPWM();
        UART1_WriteString("OK OFF\r\n");
    } else if (StringEqualsIgnoreCase(command, "STATUS") != 0U) {
        SendStatus();
    } else if (ParsePWM(command, &new_percent) != 0U) {
        pwm_percent = new_percent;
        ApplyPWM();
        UART1_WriteString("OK PWM:");
        UART1_WriteU32(pwm_percent);
        UART1_WriteString("%\r\n");
    } else {
        UART1_WriteString("ERROR: LENH KHONG HOP LE\r\n");
    }
}

void USART1_IRQHandler(void)
{
    const uint32_t status = USART1->SR;

    if ((status & USART_SR_RXNE) != 0U) {
        const uint8_t received = (uint8_t)USART1->DR;

        if (command_ready == 0U) {
            if (received == (uint8_t)'!') {
                command_buffer[command_length] = '\0';
                command_ready = 1U;
            } else if ((received == (uint8_t)'\r') ||
                       (received == (uint8_t)'\n')) {
                /* Ignore terminal line endings. */
            } else if (command_length < (COMMAND_BUFFER_SIZE - 1U)) {
                command_buffer[command_length++] = (char)received;
            } else {
                command_length = 0U;
            }
        }
    } else if ((status & (USART_SR_ORE | USART_SR_NE | USART_SR_FE)) != 0U) {
        (void)USART1->DR;
    }
}

int main(void)
{
    Clock_Init();
    TIM2_PWM_Init();
    UART1_Init(115200U);

    NVIC_SetPriority(USART1_IRQn, 1U);
    NVIC_EnableIRQ(USART1_IRQn);
    USART1->CR1 |= USART_CR1_RXNEIE;
    UART1_WriteString("Lenh: ON! OFF! PWM:0..100%! STATUS!\r\n");

    while (1) {
        if (command_ready != 0U) {
            char local_command[COMMAND_BUFFER_SIZE];
            uint32_t index = 0U;

            __disable_irq();
            while (index <= command_length) {
                local_command[index] = command_buffer[index];
                ++index;
            }
            command_length = 0U;
            command_ready = 0U;
            __enable_irq();
            ProcessCommand(local_command);
        } else {
            __WFI();
        }
    }
}
