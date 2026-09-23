/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Bài 03 - Button, UART và DMA
  ******************************************************************************
  * @attention
  *
  * Mã lớp  : ELE1415
  * Mã nhóm : 02
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define UART_TX_BUFFER_SIZE 64U
#define BUTTON_DEBOUNCE_MS  50U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* Cờ báo có sự kiện nhấn nút */
static volatile uint8_t button_event = 0U;

/* Cờ báo UART DMA đang truyền */
static volatile uint8_t uart_tx_busy = 0U;

/* Số lần nút được nhấn */
static uint32_t button_count = 0U;

/* Thời điểm nhận lần nhấn gần nhất */
static uint32_t last_button_tick = 0U;

/*
 * Buffer phải tồn tại trong suốt thời gian DMA truyền.
 * Không được khai báo buffer này là biến cục bộ trong while.
 */
static uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  Điểm bắt đầu chương trình.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Reset các ngoại vi, khởi tạo Flash và SysTick */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Cấu hình Clock hệ thống */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Khởi tạo các ngoại vi đã cấu hình bằng CubeMX */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */

  /*
   * Không cần khởi động DMA ở đây.
   * HAL_UART_Transmit_DMA() sẽ khởi động DMA khi có dữ liệu cần gửi.
   */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /*
     * Chỉ xử lý khi:
     * 1. Có sự kiện nhấn nút.
     * 2. UART DMA đang rảnh.
     */
    if ((button_event != 0U) && (uart_tx_busy == 0U))
    {
      int message_length;

      /* Xóa cờ sự kiện trước khi xử lý */
      button_event = 0U;

      /* Mỗi lần nhấn, giá trị tăng thêm một */
      button_count++;

      /*
       * Tạo bản tin theo yêu cầu:
       * <ID-Lớp><ID-Nhóm>:BTN:<Giá trị>\n\r
       *
       * Với:
       * ID-Lớp  = ELE1415
       * ID-Nhóm = 02
       */
      message_length = snprintf(
          (char *)uart_tx_buffer,
          sizeof(uart_tx_buffer),
          "ELE141502:BTN:%lu\n\r",
          (unsigned long)button_count
      );

      /*
       * Chỉ truyền nếu snprintf thành công và dữ liệu
       * không vượt quá kích thước buffer.
       */
      if ((message_length > 0) &&
          ((uint32_t)message_length < sizeof(uart_tx_buffer)))
      {
        uart_tx_busy = 1U;

        /*
         * HAL_UART_Transmit_DMA() là hàm không chờ.
         * DMA sẽ tự chuyển dữ liệu từ RAM sang USART1.
         */
        if (HAL_UART_Transmit_DMA(
                &huart1,
                uart_tx_buffer,
                (uint16_t)message_length
            ) != HAL_OK)
        {
          /*
           * Nếu không khởi động được DMA,
           * trả UART về trạng thái rảnh.
           */
          uart_tx_busy = 0U;
        }
      }
    }
  }

  /* USER CODE END 3 */
}

/**
  * @brief  Cấu hình Clock hệ thống.
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /*
   * HSE = 8 MHz
   * PLL = 8 MHz x 9 = 72 MHz
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
   * SYSCLK = 72 MHz
   * HCLK   = 72 MHz
   * PCLK1  = 36 MHz
   * PCLK2  = 72 MHz
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
}

/* USER CODE BEGIN 4 */

/**
  * @brief Hàm callback được gọi khi xảy ra ngắt ngoài GPIO.
  *
  * Nút nhấn được nối:
  * PA0 ---- nút nhấn ---- GND
  *
  * PA0 được cấu hình Input Pull-up và ngắt cạnh xuống.
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == BTN_Pin)
  {
    uint32_t current_tick;

    current_tick = HAL_GetTick();

    /*
     * Chống dội nút trong 50 ms.
     * Các cạnh phát sinh trong khoảng 50 ms sẽ bị bỏ qua.
     */
    if ((current_tick - last_button_tick) >= BUTTON_DEBOUNCE_MS)
    {
      last_button_tick = current_tick;
      button_event = 1U;
    }
  }
}

/**
  * @brief Callback được gọi sau khi UART DMA truyền xong.
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    /* Cho phép gửi bản tin tiếp theo */
    uart_tx_busy = 0U;
  }
}

/* USER CODE END 4 */

/**
  * @brief  Hàm xử lý khi xảy ra lỗi.
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
