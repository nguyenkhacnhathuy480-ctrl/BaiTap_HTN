#include "motor_driver.h"
#include "board.h"
#include "robot_config.h"
static uint8_t direction_valid,last_direction;
static int32_t clamp(int32_t x)
{
    return x > ROBOT_PWM_LIMIT ? ROBOT_PWM_LIMIT : (x < -ROBOT_PWM_LIMIT ? -ROBOT_PWM_LIMIT : x);
}
void Motors_Init(void)
{
    Board_EmergencyStop();
    Board_Output(BOARD_LEFT_IN1_PORT, BOARD_LEFT_IN1_PIN);
    Board_Output(BOARD_LEFT_IN2_PORT, BOARD_LEFT_IN2_PIN);
    Board_Output(BOARD_RIGHT_IN1_PORT, BOARD_RIGHT_IN1_PIN);
    Board_Output(BOARD_RIGHT_IN2_PORT, BOARD_RIGHT_IN2_PIN);
    GPIOA->CRH = (GPIOA->CRH & ~255U) | 0xBBU;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    /* APB2 /1 in both the HSI diagnostic and 72 MHz robot configurations. */
    TIM1->PSC = 0U; TIM1->ARR = SystemCoreClock / BOARD_PWM_HZ - 1U;
    TIM1->CCMR1 = (6U << 4U) | TIM_CCMR1_OC1PE | (6U << 12U) | TIM_CCMR1_OC2PE;
    TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;
    TIM1->BDTR = TIM_BDTR_MOE;
    TIM1->CR1 = TIM_CR1_ARPE;
    Motors_Stop(); TIM1->EGR = TIM_EGR_UG; TIM1->CR1 |= TIM_CR1_CEN;
}
void Motors_Stop(void)
{
    Board_Write(BOARD_STBY_PORT, BOARD_STBY_PIN, 0U);
    TIM1->CCR1 = 0U; TIM1->CCR2 = 0U;
    direction_valid=0U;
}
void Motors_Set(int32_t left, int32_t right)
{
    uint32_t lmag, rmag, period = TIM1->ARR + 1U;
    uint8_t direction,changed;
    left = clamp(left); right = clamp(right);
    if (BOARD_LEFT_MOTOR_REVERSED) left = -left;
    if (BOARD_RIGHT_MOTOR_REVERSED) right = -right;
    if (left == 0 && right == 0) { Motors_Stop(); return; }
    direction=(uint8_t)((left>0?1U:0U)|(left<0?2U:0U)|
                       (right>0?4U:0U)|(right<0?8U:0U));
    changed=(uint8_t)(!direction_valid || direction!=last_direction);
    if (changed) {
        Board_Write(BOARD_STBY_PORT, BOARD_STBY_PIN, 0U);
        Board_Write(BOARD_LEFT_IN1_PORT, BOARD_LEFT_IN1_PIN, left > 0);
        Board_Write(BOARD_LEFT_IN2_PORT, BOARD_LEFT_IN2_PIN, left < 0);
        Board_Write(BOARD_RIGHT_IN1_PORT, BOARD_RIGHT_IN1_PIN, right > 0);
        Board_Write(BOARD_RIGHT_IN2_PORT, BOARD_RIGHT_IN2_PIN, right < 0);
    }
    lmag = (uint32_t)(left < 0 ? -left : left); rmag = (uint32_t)(right < 0 ? -right : right);
    /* Duty units always per-mille. The safety cap must NOT rescale to 100%. */
    TIM1->CCR1 = lmag * period / ROBOT_DUTY_SCALE;
    TIM1->CCR2 = rmag * period / ROBOT_DUTY_SCALE;
    /* Normal duty updates latch at the next PWM update event. Do not pulse
     * STBY or reset the PWM counter on every 1 ms control iteration. */
    if (changed) {
        TIM1->EGR = TIM_EGR_UG;
        last_direction=direction;direction_valid=1U;
        Board_Write(BOARD_STBY_PORT, BOARD_STBY_PIN, 1U);
    }
}
