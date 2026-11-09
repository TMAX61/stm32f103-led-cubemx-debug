/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
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
#define DUTY_STEP      10
#define DUTY_MIN       20
#define DUTY_MAX       80
#define AUTO_STEP_MS   20
#define RX_SIZE        64
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
typedef enum { MODE_MANUAL = 0, MODE_AUTO } mode_t;

mode_t   g_mode = MODE_MANUAL;
uint16_t g_duty = 0;

static int8_t   auto_dir  = 1;
static uint32_t auto_tick = 0;

static uint8_t rx_byte;
static uint8_t rx_buf[RX_SIZE];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static uint8_t Uart_GetChar(uint8_t *c);
static void Report_Status(void);
static void Report_Error(const char *msg);
static void Apply_Duty(void);
static void Enter_Auto(void);
static void Handle_Command(uint8_t ch);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  uint8_t ch;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  Apply_Duty();
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  Report_Status();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    while (Uart_GetChar(&ch))
    {
      Handle_Command(ch);
    }

    if (g_mode == MODE_AUTO && (HAL_GetTick() - auto_tick) >= AUTO_STEP_MS)
    {
      auto_tick += AUTO_STEP_MS;
      if ((HAL_GetTick() - auto_tick) >= AUTO_STEP_MS)
      {
        auto_tick = HAL_GetTick();
      }

      if (auto_dir > 0)
      {
        g_duty++;
        if (g_duty >= DUTY_MAX) { g_duty = DUTY_MAX; auto_dir = -1; }
      }
      else
      {
        g_duty--;
        if (g_duty <= DUTY_MIN) { g_duty = DUTY_MIN; auto_dir = 1; }
      }
      Apply_Duty();
    }

    HAL_Delay(1);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
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

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

  /* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    uint8_t next = (uint8_t)((rx_head + 1) % RX_SIZE);
    if (next != rx_tail)
    {
      rx_buf[rx_head] = rx_byte;
      rx_head = next;
    }
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  }
}

static uint8_t Uart_GetChar(uint8_t *c)
{
  if (rx_tail == rx_head) return 0;
  *c = rx_buf[rx_tail];
  rx_tail = (uint8_t)((rx_tail + 1) % RX_SIZE);
  return 1;
}

static void Report_Status(void)
{
  char buf[48];
  int n = sprintf(buf, "Mode: %s\r\nDuty: %d\r\n\r\n",
                  (g_mode == MODE_AUTO) ? "Auto" : "Manual", (int)g_duty);
  HAL_UART_Transmit(&huart1, (uint8_t *)buf, (uint16_t)n, 100);
}

static void Report_Error(const char *msg)
{
  char buf[64];
  int n = sprintf(buf, "Error: %s\r\n\r\n", msg);
  HAL_UART_Transmit(&huart1, (uint8_t *)buf, (uint16_t)n, 100);
}

static void Apply_Duty(void)
{
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, (uint32_t)g_duty * 65535u / 100u);
}

static void Enter_Auto(void)
{
  g_mode = MODE_AUTO;
  if (g_duty < DUTY_MIN) g_duty = DUTY_MIN;
  if (g_duty > DUTY_MAX) g_duty = DUTY_MAX;
  auto_dir  = (g_duty >= 50) ? -1 : 1;
  auto_tick = HAL_GetTick();
  Apply_Duty();
}

static void Handle_Command(uint8_t ch)
{
  switch (ch)
  {
  case '+':
  case '-':
    if (g_mode == MODE_AUTO)
    {
      Report_Error("'+'/'-' not allowed in Auto mode");
      return;
    }
    if (ch == '+')
      g_duty = (g_duty + DUTY_STEP > 100) ? 100 : g_duty + DUTY_STEP;
    else
      g_duty = (g_duty < DUTY_STEP) ? 0 : g_duty - DUTY_STEP;
    Apply_Duty();
    Report_Status();
    break;

  case 'A':
  case 'a':
    Enter_Auto();
    Report_Status();
    break;

  case 'M':
  case 'm':
    g_mode = MODE_MANUAL;
    Apply_Duty();
    Report_Status();
    break;

  case '\r':
  case '\n':
  case ' ':
  case '\t':
    break;

  default:
    Report_Error("unknown command");
    break;
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
