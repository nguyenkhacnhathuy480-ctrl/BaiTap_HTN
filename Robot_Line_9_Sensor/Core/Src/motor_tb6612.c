#include "board_config.h"
#include "motor_tb6612.h"

static uint32_t ClampMagnitude(int32_t speed)
{
    if (speed < 0) {
        speed = -speed;
    }
    if (speed > MOTOR_PWM_MAX) {
        speed = MOTOR_PWM_MAX;
    }
    return (uint32_t)speed;
}

static void SetPin(GPIO_TypeDef *port, uint32_t pin, uint8_t high)
{
    if (high != 0U) {
        port->BSRR = (1U << pin);
    } else {
        port->BRR = (1U << pin);
    }
}

static void SetDirection(int32_t speed, GPIO_TypeDef *in1_port, uint32_t in1_pin,
                         GPIO_TypeDef *in2_port, uint32_t in2_pin)
{
    if (speed > 0) {
        SetPin(in1_port, in1_pin, 1U);
        SetPin(in2_port, in2_pin, 0U);
    } else if (speed < 0) {
        SetPin(in1_port, in1_pin, 0U);
        SetPin(in2_port, in2_pin, 1U);
    } else {
        SetPin(in1_port, in1_pin, 0U);
        SetPin(in2_port, in2_pin, 0U);
    }
}

void Motor_Init(void)
{
    uint32_t crh;

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN |
                    RCC_APB2ENR_TIM1EN;

    /* PA8/PA9: TIM1_CH1/TIM1_CH2 alternate-function push-pull, 50 MHz. */
    crh = GPIOA->CRH;
    crh &= ~0xFFU;
    crh |= 0xBBU;
    GPIOA->CRH = crh;

    /* PB11..PB15: STBY and four direction outputs, push-pull 2 MHz. */
    crh = GPIOB->CRH;
    crh &= ~(0xFFFFFU << 12U);
    crh |= (0x22222U << 12U);
    GPIOB->CRH = crh;

    TIM1->PSC = 0U;
    TIM1->ARR = (BOARD_SYSTEM_CLOCK_HZ / MOTOR_PWM_FREQUENCY_HZ) - 1U;
    TIM1->CCR1 = 0U;
    TIM1->CCR2 = 0U;
    TIM1->CCMR1 = TIM_CCMR1_OC1PE | (6U << TIM_CCMR1_OC1M_Pos) |
                   TIM_CCMR1_OC2PE | (6U << TIM_CCMR1_OC2M_Pos);
    TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;
    TIM1->BDTR = TIM_BDTR_MOE;
    TIM1->CR1 = TIM_CR1_ARPE;
    TIM1->EGR = TIM_EGR_UG;
    TIM1->CR1 |= TIM_CR1_CEN;

    Motor_Stop();
    SetPin(TB6612_STBY_PORT, TB6612_STBY_PIN, 1U);
}

void Motor_Set(int32_t left_speed, int32_t right_speed)
{
    const uint32_t period = TIM1->ARR + 1U;
    const uint32_t left_duty = (ClampMagnitude(left_speed) * period) / MOTOR_PWM_MAX;
    const uint32_t right_duty = (ClampMagnitude(right_speed) * period) / MOTOR_PWM_MAX;

    SetDirection(left_speed, TB6612_AIN1_PORT, TB6612_AIN1_PIN,
                 TB6612_AIN2_PORT, TB6612_AIN2_PIN);
    SetDirection(right_speed, TB6612_BIN1_PORT, TB6612_BIN1_PIN,
                 TB6612_BIN2_PORT, TB6612_BIN2_PIN);
    TIM1->CCR1 = left_duty;
    TIM1->CCR2 = right_duty;
}

void Motor_Stop(void)
{
    TIM1->CCR1 = 0U;
    TIM1->CCR2 = 0U;
    SetDirection(0, TB6612_AIN1_PORT, TB6612_AIN1_PIN,
                 TB6612_AIN2_PORT, TB6612_AIN2_PIN);
    SetDirection(0, TB6612_BIN1_PORT, TB6612_BIN1_PIN,
                 TB6612_BIN2_PORT, TB6612_BIN2_PIN);
}
