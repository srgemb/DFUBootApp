/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define ADDR_APP_SIZE       0x0000001C  //смещение для значения размера приложения
#define ADDR_APP_CRC        0x00000020  //смещение для значения контрольной суммы

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

SD_HandleTypeDef hsd;

TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
typedef void (*pFunction)(void);
pFunction JumpToApplication;
uint32_t JumpAddress, Address;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SDIO_SD_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM6_Init(void);
static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint32_t AddrAppBegin = USBD_DFU_APP_DEFAULT_ADD;
uint32_t AddrAppEnd = 0x00;
uint32_t AddrAppSize, AddrAppCrc, AppSize, AppCrc, ValueSp;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main( void ) {

    /* USER CODE BEGIN 1 */
    uint32_t crc_flash;
    char *ptr, str[160];
    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */
    AddrAppSize = USBD_DFU_APP_DEFAULT_ADD + ADDR_APP_SIZE;
    AddrAppCrc = USBD_DFU_APP_DEFAULT_ADD + ADDR_APP_CRC;
    AppSize = *(__IO uint32_t*) ( USBD_DFU_APP_DEFAULT_ADD + ADDR_APP_SIZE);
    AppCrc = *(__IO uint32_t*) ( USBD_DFU_APP_DEFAULT_ADD + ADDR_APP_CRC);
    AddrAppEnd = USBD_DFU_APP_DEFAULT_ADD + AppSize;
    if ( (int32_t)AppSize < 0 )
        AddrAppEnd = USBD_DFU_APP_DEFAULT_ADD + 0x00060000;
    ValueSp = *(__IO uint32_t*) USBD_DFU_APP_DEFAULT_ADD;
    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_SDIO_SD_Init();
    MX_USB_DEVICE_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_TIM6_Init();
    MX_CRC_Init();

    /* USER CODE BEGIN 2 */
    HAL_GPIO_WritePin( LED2_GPIO_Port, LED1_Pin, GPIO_PIN_SET );
    ptr = str;
    ptr += sprintf( ptr, "\rApplication address: 0x%08X\r", (unsigned)AddrAppBegin );
    ptr += sprintf( ptr, "Application size:    0x%08X => 0x%08X (%d)\r", (unsigned)AddrAppSize, (unsigned)AppSize, (unsigned)AppSize );
    ptr += sprintf( ptr, "Application CRC:     0x%08X => 0x%08X\r", (unsigned)AddrAppCrc, (unsigned)AppCrc );
    HAL_UART_Transmit( &huart2, (uint8_t*) str, strlen( str ), HAL_MAX_DELAY );

    if ( (int32_t)AppSize > 0 ) {
        ptr = str;
        HAL_CRC_Calculate( &hcrc, (uint32_t*) USBD_DFU_APP_DEFAULT_ADD, ADDR_APP_SIZE / sizeof(uint32_t) );
        crc_flash = HAL_CRC_Accumulate( &hcrc, (uint32_t*) USBD_DFU_APP_DEFAULT_ADD + ADDR_APP_CRC / sizeof(uint32_t) + 1, (AppSize / sizeof(uint32_t)) - 9 );
        ptr += sprintf( ptr, "\rCalculate CRC: 0x%08X-0x%08X, ", USBD_DFU_APP_DEFAULT_ADD, USBD_DFU_APP_DEFAULT_ADD + ADDR_APP_SIZE - 1 );
        ptr += sprintf( ptr, "0x%08X-0x%08X = 0x%08X\r", (unsigned)( USBD_DFU_APP_DEFAULT_ADD + ADDR_APP_CRC + sizeof(uint32_t) ),
                (unsigned)( USBD_DFU_APP_DEFAULT_ADD + AppSize - ADDR_APP_CRC + sizeof(uint32_t) ), (unsigned)crc_flash );
        HAL_UART_Transmit( &huart2, (uint8_t*) str, strlen( str ), HAL_MAX_DELAY );
    }
    //Check if the KEY Button is pressed
    if ( HAL_GPIO_ReadPin( BOOT1_GPIO_Port, BOOT1_Pin ) == GPIO_PIN_RESET ) {
        //Test if user code is programmed starting from address 0x0800C000
        if ( (ValueSp & 0x2FFE0000) == 0x20000000 && (int32_t)AppSize > 0 && AppCrc == crc_flash ) {
            sprintf( str, "Checksum - OK\r" );
            HAL_UART_Transmit( &huart2, (uint8_t*)str, strlen( str ), HAL_MAX_DELAY );
            HAL_GPIO_WritePin( LED2_GPIO_Port, LED1_Pin, GPIO_PIN_RESET );
            /* Jump to user application */
            JumpAddress = *(__IO uint32_t*) ( USBD_DFU_APP_DEFAULT_ADD + 4);
            JumpToApplication = (pFunction) JumpAddress;
            sprintf( str, "Start application: 0x%08X\r", (unsigned)JumpAddress );
            HAL_UART_Transmit( &huart2, (uint8_t*) str, strlen( str ), HAL_MAX_DELAY );
            HAL_GPIO_WritePin( LED2_GPIO_Port, LED1_Pin, GPIO_PIN_RESET );
            //Initialize user application's Stack Pointer
            HAL_RCC_DeInit();
            HAL_DeInit();
            __set_MSP( *(__IO uint32_t*) USBD_DFU_APP_DEFAULT_ADD );
            JumpToApplication();
        }
        else {
            ptr = str;
            ptr += sprintf( ptr, "Application not found.\r" );
            ptr += sprintf( ptr, "\rDFU mode ...\r" );
            HAL_UART_Transmit( &huart2, (uint8_t*) str, strlen( str ), HAL_MAX_DELAY );
            HAL_TIM_Base_Start_IT( &htim6 );
        }
    }
    else {
        sprintf( str, "\rDFU mode ...\r" );
        HAL_UART_Transmit( &huart2, (uint8_t*) str, strlen( str ), HAL_MAX_DELAY );
        HAL_TIM_Base_Start_IT( &htim6 );
    }
    MX_USB_DEVICE_Init();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while ( 1 ) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDIO_SD_Init(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 0;
  if (HAL_SD_Init(&hsd) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SDIO_Init 2 */

  /* USER CODE END SDIO_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 9999;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 3599;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED1_Pin|LED2_Pin|LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SD_Power_GPIO_Port, SD_Power_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_DISCONN_GPIO_Port, USB_DISCONN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED1_Pin LED2_Pin LED3_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SD_Power_Pin USB_DISCONN_Pin */
  GPIO_InitStruct.Pin = SD_Power_Pin|USB_DISCONN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_Detect_Pin */
  GPIO_InitStruct.Pin = SD_Detect_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SD_Detect_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
