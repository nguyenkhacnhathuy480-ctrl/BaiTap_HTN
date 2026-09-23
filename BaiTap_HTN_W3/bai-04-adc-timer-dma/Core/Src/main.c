/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Week 03 - Bài 04
  *                   ADC + Timer Trigger + DMA + UART
  ******************************************************************************
  * @attention
  *
  * Mã lớp  : ELE1415
  * Mã nhóm : 02
  *
  * TIM3 tạo trigger 100 Hz.
  * ADC1 đọc PA1.
  * DMA lưu 100 mẫu ADC trong 1 giây.
  * Half-transfer gửi mẫu 0..49.
  * Transfer-complete gửi mẫu 50..99.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <stdint.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* TIM3 lấy mẫu ADC với tần số 100 Hz */
#define ADC_SAMPLE_RATE_HZ       100U

/* Tổng số mẫu thu trong một giây */
#define ADC_BUFFER_SIZE          100U

/* Mỗi nửa buffer chứa 50 mẫu */
#define ADC_HALF_BUFFER_SIZE      50U

/* Đủ chứa 50 giá trị ADC dạng "4095\n\r" */
#define UART_TX_BUFFER_SIZE      512U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/*
 * Buffer ADC DMA.
 *
 * ADC STM32F103 có độ phân giải 12 bit: 0..4095.
 * Mỗi mẫu được lưu bằng uint16_t.
 */
static uint16_t adc_buffer[ADC_BUFFER_SIZE];

/*
 * Buffer chứa chuỗi UART.
 *
 * Buffer phải tồn tại trong toàn bộ thời gian UART DMA truyền,
 * vì vậy phải khai báo static/toàn cục.
 */
static uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE];

/* Cờ báo DMA ADC đã ghi xong nửa đầu buffer */
static volatile uint8_t adc_half_ready = 0U;

/* Cờ báo DMA ADC đã ghi xong nửa cuối buffer */
static volatile uint8_t adc_full_ready = 0U;

/* Cờ báo UART DMA đang bận truyền */
static volatile uint8_t uart_tx_busy = 0U;

/* Cờ báo lỗi ADC, phục vụ kiểm tra khi debug */
static volatile uint8_t adc_error = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

/**
  * @brief Chuyển 50 mẫu ADC thành chuỗi và gửi bằng UART DMA.
  * @param start_index Vị trí đầu tiên của nửa buffer cần gửi.
  */
static void UART_Send_ADC_Block(uint32_t start_index);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief Chuyển một nửa buffer ADC thành chuỗi rồi truyền UART DMA.
  *
  * Mỗi giá trị được truyền theo yêu cầu:
  *
  * 1234\n\r
  * 1235\n\r
  * 1236\n\r
  *
  * @param start_index:
  *        0  -> gửi adc_buffer[0..49]
  *        50 -> gửi adc_buffer[50..99]
  */
static void UART_Send_ADC_Block(uint32_t start_index)
{
  uint32_t adc_index;
  uint32_t used = 0U;

  for (adc_index = start_index;
       adc_index < (start_index + ADC_HALF_BUFFER_SIZE);
       adc_index++)
  {
    int written;
    uint32_t remaining;

    remaining = UART_TX_BUFFER_SIZE - used;

    written = snprintf(
        (char *)&uart_tx_buffer[used],
        (size_t)remaining,
        "%u\n\r",
        (unsigned int)adc_buffer[adc_index]
    );

    /*
     * snprintf trả về số ký tự muốn ghi, không tính '\0'.
     * Nếu written >= remaining thì buffer không đủ chỗ.
     */
    if ((written <= 0) || ((uint32_t)written >= remaining))
    {
      return;
    }

    used += (uint32_t)written;
  }

  if (used > 0U)
  {
    uart_tx_busy = 1U;

    /*
     * Khởi động UART DMA.
     * Hàm trả về ngay, CPU không phải chờ UART truyền xong.
     */
    if (HAL_UART_Transmit_DMA(
            &huart1,
            uart_tx_buffer,
            (uint16_t)used
        ) != HAL_OK)
    {
      uart_tx_busy = 0U;
    }
  }
}

/* USER CODE END 0 */

/**
  * @brief  Điểm bắt đầu chương trình.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /*
   * Reset ngoại vi, khởi tạo Flash và SysTick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /*
   * Cấu hình Clock:
   * SYSCLK = 72 MHz
   * ADC Clock = 12 MHz
   */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /*
   * Khởi tạo các ngoại vi đã cấu hình trong CubeMX.
   */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */

  /*
   * Hiệu chuẩn ADC trước khi bắt đầu lấy mẫu.
   */
  if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /*
   * Khởi động ADC và DMA trước.
   *
   * DMA lưu 100 mẫu vào adc_buffer.
   * DMA phải được cấu hình Circular trong CubeMX.
   */
  if (HAL_ADC_Start_DMA(
          &hadc1,
          (uint32_t *)adc_buffer,
          ADC_BUFFER_SIZE
      ) != HAL_OK)
  {
    Error_Handler();
  }

  /*
   * Khởi động TIM3 sau khi ADC DMA đã sẵn sàng.
   *
   * TIM3 phát TRGO 100 lần mỗi giây.
   */
  if (HAL_TIM_Base_Start(&htim3) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /*
     * Callback ngắt ADC chỉ đặt cờ.
     * Việc định dạng chuỗi và khởi động UART DMA được xử lý
     * trong vòng lặp chính để callback ngắt chạy thật nhanh.
     */

    if ((adc_half_ready != 0U) && (uart_tx_busy == 0U))
    {
      /*
       * DMA ADC đã ghi xong mẫu 0..49.
       * Trong khi gửi nửa này, DMA tiếp tục ghi mẫu 50..99.
       */
      adc_half_ready = 0U;
      UART_Send_ADC_Block(0U);
    }
    else if ((adc_full_ready != 0U) && (uart_tx_busy == 0U))
    {
      /*
       * DMA ADC đã ghi xong mẫu 50..99.
       * Trong khi gửi nửa này, DMA quay lại ghi mẫu 0..49.
       */
      adc_full_ready = 0U;
      UART_Send_ADC_Block(ADC_HALF_BUFFER_SIZE);
    }

    /*
     * Nếu adc_error khác 0, ADC đã xảy ra lỗi.
     * Có thể đặt breakpoint ở đây khi debug.
     */
    if (adc_error != 0U)
    {
      Error_Handler();
    }
  }

  /* USER CODE END 3 */
}

/**
  * @brief Cấu hình Clock hệ thống.
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /*
   * HSE = 8 MHz.
   * PLL = HSE x 9 = 72 MHz.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;

  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /*
   * SYSCLK = 72 MHz.
   * HCLK   = 72 MHz.
   * PCLK1  = 36 MHz.
   * PCLK2  = 72 MHz.
   *
   * Timer APB1 Clock = 72 MHz vì APB1 Prescaler khác 1.
   */
  RCC_ClkInitStruct.ClockType =
      RCC_CLOCKTYPE_HCLK |
      RCC_CLOCKTYPE_SYSCLK |
      RCC_CLOCKTYPE_PCLK1 |
      RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(
          &RCC_ClkInitStruct,
          FLASH_LATENCY_2
      ) != HAL_OK)
  {
    Error_Handler();
  }

  /*
   * ADC Clock = PCLK2 / 6
   *           = 72 MHz / 6
   *           = 12 MHz.
   *
   * ADC STM32F103 không được chạy quá 14 MHz.
   */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
  * @brief Callback DMA ADC Half-transfer.
  *
  * Hàm được gọi khi DMA đã ghi xong:
  * adc_buffer[0] đến adc_buffer[49].
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1)
  {
    adc_half_ready = 1U;
  }
}

/**
  * @brief Callback DMA ADC Transfer-complete.
  *
  * Hàm được gọi khi DMA đã ghi xong:
  * adc_buffer[50] đến adc_buffer[99].
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1)
  {
    adc_full_ready = 1U;
  }
}

/**
  * @brief Callback khi UART DMA đã truyền xong.
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    uart_tx_busy = 0U;
  }
}

/**
  * @brief Callback khi ADC xảy ra lỗi.
  */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1)
  {
    adc_error = 1U;
  }
}

/* USER CODE END 4 */

/**
  * @brief Hàm xử lý lỗi.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */

  __disable_irq();

  while (1)
  {
  }

  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT

/**
  * @brief Báo tên file và dòng xảy ra lỗi tham số.
  * @param file Tên file nguồn.
  * @param line Dòng xảy ra lỗi.
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */

  (void)file;
  (void)line;

  /* USER CODE END 6 */
}

#endif /* USE_FULL_ASSERT */
