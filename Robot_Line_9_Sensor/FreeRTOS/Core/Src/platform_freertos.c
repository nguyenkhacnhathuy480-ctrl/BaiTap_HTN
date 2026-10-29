#include "board_config.h"
#include "FreeRTOS.h"
#include "platform.h"
#include "task.h"

void Platform_Init(void)
{
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) == 0U) {
    }
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2 |
                   RCC_CFGR_ADCPRE | RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE |
                   RCC_CFGR_PLLMULL | RCC_CFGR_SW);
    RCC->CFGR |= RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9 |
                 RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6;
    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0U) {
    }
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
    }
    SystemCoreClock = BOARD_SYSTEM_CLOCK_HZ;

    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH = (GPIOC->CRH & ~(0xFU << 20U)) | (0x2U << 20U);
    GPIOC->BSRR = (1U << STATUS_LED_PIN);
}

uint32_t Platform_Millis(void)
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

void Platform_WaitUntil(uint32_t timestamp_ms)
{
    const uint32_t now = Platform_Millis();
    if ((int32_t)(timestamp_ms - now) > 0) {
        vTaskDelay(pdMS_TO_TICKS(timestamp_ms - now));
    }
}

void Platform_StatusLedToggle(void)
{
    GPIOC->ODR ^= (1U << STATUS_LED_PIN);
}
