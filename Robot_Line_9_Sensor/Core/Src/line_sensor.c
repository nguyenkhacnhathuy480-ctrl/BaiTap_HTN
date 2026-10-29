#include "board_config.h"
#include "line_sensor.h"

static const int16_t sensor_weight[LINE_SENSOR_COUNT] = {
    -4000, -3000, -2000, -1000, 0, 1000, 2000, 3000, 4000
};

void LineSensor_Init(void)
{
    uint32_t crl_a;

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;

    /* PA0..PA7 and PB0: input pull-up. */
    crl_a = GPIOA->CRL;
    crl_a &= ~0xFFFFFFFFU;
    crl_a |= 0x88888888U;
    GPIOA->CRL = crl_a;
    GPIOA->BSRR = LINE_SENSOR_PORT_A_MASK;

    GPIOB->CRL = (GPIOB->CRL & ~0xFU) | 0x8U;
    GPIOB->BSRR = (1U << LINE_SENSOR_PORT_B_PIN);
}

uint16_t LineSensor_ReadMask(void)
{
    uint16_t raw_mask;
    uint16_t active_mask;

    raw_mask = (uint16_t)(GPIOA->IDR & LINE_SENSOR_PORT_A_MASK);
    raw_mask |= (uint16_t)(((GPIOB->IDR >> LINE_SENSOR_PORT_B_PIN) & 1U) << 8U);

#if LINE_SENSOR_ACTIVE_LOW
    active_mask = (uint16_t)(~raw_mask) & 0x01FFU;
#else
    active_mask = raw_mask & 0x01FFU;
#endif
    return active_mask;
}

uint8_t LineSensor_GetPosition(int32_t *position)
{
    const uint16_t active_mask = LineSensor_ReadMask();
    int32_t weighted_sum = 0;
    uint32_t active_count = 0U;
    uint32_t index;

    for (index = 0U; index < LINE_SENSOR_COUNT; ++index) {
        if ((active_mask & (1U << index)) != 0U) {
            weighted_sum += sensor_weight[index];
            ++active_count;
        }
    }

    if (active_count == 0U) {
        return 0U;
    }

    *position = weighted_sum / (int32_t)active_count;
    return 1U;
}
