/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SHT3X_I2C_ADDR_44 (0x44u << 1) /* ADDR pin = GND */
#define SHT3X_I2C_ADDR_45 (0x45u << 1) /* ADDR pin = VDD */
#ifndef SHT3X_I2C_ADDR
#define SHT3X_I2C_ADDR SHT3X_I2C_ADDR_44
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
CAN_TxHeaderTypeDef CAN_pHeader;
CAN_RxHeaderTypeDef CAN_pHeaderRx;
CAN_FilterTypeDef CAN_sFilterConfig;
uint32_t CAN_pTxMailbox;
uint8_t CAN_DATA_TX[8] = {0};
uint8_t CAN_DATA_RX[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan){

		HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &CAN_pHeaderRx, CAN_DATA_RX);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET);


}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Single-shot measurement, no clock stretching, high repeatability: 0x2400 */
static const uint8_t SHT3X_CMD_MEAS_HIGH[2] = { 0x24, 0x00 };
/* Optional: read status register 0xF32D */
static const uint8_t SHT3X_CMD_READ_STATUS[2] = { 0xF3, 0x2D };
/* CRC-8 properties: poly 0x31, init 0xFF */
static uint8_t sht3x_crc8(const uint8_t *data, uint8_t len)
{
uint8_t crc = 0xFF;
for (uint8_t i = 0; i < len; ++i) {
crc ^= data[i];
for (uint8_t b = 0; b < 8; ++b) {
if (crc & 0x80) crc = (crc << 1) ^ 0x31; else crc <<= 1;
}
}
return crc;
}


/* Read one high-repeatability measurement; returns HAL status. */
static HAL_StatusTypeDef SHT3X_Read(float *t_c, float *rh, uint8_t *crc_ok_flags)
{
uint8_t rx[6];
HAL_StatusTypeDef st;


/* Trigger single measurement */
st = HAL_I2C_Master_Transmit(&hi2c1, SHT3X_I2C_ADDR, (uint8_t*)SHT3X_CMD_MEAS_HIGH, 2, HAL_MAX_DELAY);
if (st != HAL_OK) return st;


/* Wait for conversion: high repeatability typ 12.5ms, max 15ms */
HAL_Delay(15);


/* Read 6 bytes: T_MSB, T_LSB, T_CRC, RH_MSB, RH_LSB, RH_CRC */
st = HAL_I2C_Master_Receive(&hi2c1, SHT3X_I2C_ADDR, rx, sizeof(rx), HAL_MAX_DELAY);
if (st != HAL_OK) return st;


uint8_t crc_t_ok = (sht3x_crc8(rx, 2) == rx[2]);
uint8_t crc_rh_ok = (sht3x_crc8(&rx[3], 2) == rx[5]);
if (crc_ok_flags) *crc_ok_flags = (crc_t_ok ? 0x01 : 0) | (crc_rh_ok ? 0x02 : 0);


uint16_t raw_t = ((uint16_t)rx[0] << 8) | rx[1];
uint16_t raw_rh = ((uint16_t)rx[3] << 8) | rx[4];


/* Convert to physical values */
if (t_c) *t_c = -45.0f + (175.0f * (float)raw_t / 65535.0f);
if (rh) *rh = 100.0f * (float)raw_rh / 65535.0f;


return HAL_OK;
}


static HAL_StatusTypeDef SHT3X_ReadStatus(uint16_t *status)
{
HAL_StatusTypeDef st;
uint8_t rx[3];
st = HAL_I2C_Master_Transmit(&hi2c1, SHT3X_I2C_ADDR, (uint8_t*)SHT3X_CMD_READ_STATUS, 2, HAL_MAX_DELAY);
if (st != HAL_OK) return st;
st = HAL_I2C_Master_Receive(&hi2c1, SHT3X_I2C_ADDR, rx, sizeof(rx), HAL_MAX_DELAY);
if (st != HAL_OK) return st;
if (sht3x_crc8(rx, 2) != rx[2]) return HAL_ERROR;
if (status) *status = ((uint16_t)rx[0] << 8) | rx[1];
return HAL_OK;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_CAN_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_CAN_ConfigFilter(&hcan,&CAN_sFilterConfig);
  HAL_CAN_Start(&hcan);
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO1_MSG_PENDING);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  float t, h;
  uint8_t crc_flags;
  uint16_t status = 0;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (SHT3X_Read(&t, &h, &crc_flags) == HAL_OK) {
		  if (SHT3X_ReadStatus(&status) != HAL_OK) status = 0;
		  int temp_i = (int)lroundf(t);
		  int hum_i = (int)lroundf(h);

//		  if (hum_i < 0) hum_i = 0; if (hum_i > 100) hum_i = 100;
//		  if (temp_i < -128) temp_i = -128; if (temp_i > 127) temp_i = 127;

		  CAN_DATA_TX[0] = (uint8_t)((int8_t)temp_i);
		  CAN_DATA_TX[1] = (uint8_t)hum_i;
		  CAN_DATA_TX[2] = crc_flags;

		  HAL_CAN_AddTxMessage(&hcan, &CAN_pHeader, CAN_DATA_TX, &CAN_pTxMailbox);

	  }

	  	  	  HAL_Delay(2000);
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

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */
	CAN_pHeader.StdId = 0x01;
		CAN_pHeader.DLC = 8;
		CAN_pHeader.IDE = CAN_ID_STD;
		CAN_pHeader.RTR = CAN_RTR_DATA;
  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 18;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */
  CAN_sFilterConfig.FilterActivation = CAN_FILTER_ENABLE;
    CAN_sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO1;
    CAN_sFilterConfig.SlaveStartFilterBank = 0;
    CAN_sFilterConfig.FilterBank = 8;
    CAN_sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    CAN_sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    CAN_sFilterConfig.FilterIdHigh = 0x002 << 5;
    CAN_sFilterConfig.FilterIdLow = 0;
    CAN_sFilterConfig.FilterMaskIdHigh = 0;
    CAN_sFilterConfig.FilterMaskIdLow = 0;
  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
