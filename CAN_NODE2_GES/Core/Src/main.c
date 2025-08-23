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
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define I2C_TIMEOUT    100
#define PAJ_ADDR (0x73 << 1)
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
uint8_t CAN_DATA_TX[8] = {0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02};
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
static HAL_StatusTypeDef paj_write(uint8_t reg, uint8_t data) {
    return HAL_I2C_Mem_Write(&hi2c1, PAJ_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, I2C_TIMEOUT);
}
static HAL_StatusTypeDef paj_read(uint8_t reg, uint8_t *data) {
    return HAL_I2C_Mem_Read(&hi2c1, PAJ_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_TIMEOUT);
}
static void paj_select_bank(uint8_t bank) { // bank: 0 hoặc 1
    // Thanh ghi chọn bank là 0xEF theo datasheet
    // :contentReference[oaicite:8]{index=8}
    paj_write(0xEF, bank);
}
const uint8_t initRegisterArray[][2	] = {
		{0xEF,0x00},
		{0x41,0xFF},
		{0x42,0x01},
		{0x46,0x2D},
		{0x47,0x0F},
		{0x48,0x80},
		{0x49,0x00},
		{0x4A,0x40},
		{0x4B,0x00},
		{0x4C,0x20},
		{0x4D,0x00},
		{0x51,0x10},
		{0x5C,0x02},
		{0x5E,0x10},
		{0x80,0x41},
		{0x81,0x44},
		{0x82,0x0C},
		{0x83,0x20},
		{0x84,0x20},
		{0x85,0x00},
		{0x86,0x10},
		{0x87,0x00},
		{0x8B,0x01},
		{0x8D,0x00},
		{0x90,0x0C},
		{0x91,0x0C},
		{0x93,0x0D},
		{0x94,0x0A},
		{0x95,0x0A},
		{0x96,0x0C},
		{0x97,0x05},
		{0x9A,0x14},
		{0x9C,0x3F},
		{0x9F,0xF9},
		{0xA0,0x48},
		{0xA5,0x19},
		{0xCC,0x19},
		{0xCD,0x0B},
		{0xCE,0x13},
		{0xCF,0x62},
		{0xD0,0x21},
		{0xEF,0x01},
		{0x00,0x1E},
		{0x01,0x1E},
		{0x02,0x0F},
		{0x03,0x0F},
		{0x04,0x02},
		{0x25,0x01},
		{0x26,0x00},
		{0x27,0x39},
		{0x28,0x7F},
		{0x29,0x08},
		{0x30,0x03},
		{0x3E,0xFF},
		{0x5E,0x3D},
		{0x65,0xAC},
		{0x66,0x00},
		{0x67,0x97},
		{0x68,0x01},
		{0x69,0xCD},
		{0x6A,0x01},
		{0x6B,0xB0},
		{0x6C,0x04},
		{0x6D,0x2C},
		{0x6E,0x01},
		{0x72,0x01},
		{0x73,0x35},
		{0x74,0x00},
		{0x77,0x01},
		{0xEF,0x00},

};

typedef enum {
	GES_ERR=-1,
    GES_NONE=0, GES_FORWARD=1, GES_BACKWARD=2, GES_RIGHT=3, GES_LEFT=4,
    GES_UP=5, GES_DOWN=6, GES_CLOCKWISE=7, GES_ANTICLOCKWISE=8, GES_WAVE=9
} gesture_t;

static gesture_t paj_read_gesture() {
    uint8_t g0=0, g1=0;
    if (paj_read(0x43, &g0) != HAL_OK) return GES_ERR;
    if (paj_read(0x44, &g1) != HAL_OK) return GES_ERR;

    if (g0 & (1<<0)) return GES_RIGHT;
    if (g0 & (1<<1)) return GES_LEFT;
    if (g0 & (1<<2)) return GES_UP;
    if (g0 & (1<<3)) return GES_DOWN;
    if (g0 & (1<<4)) return GES_FORWARD;
    if (g0 & (1<<5)) return GES_BACKWARD;
    if (g0 & (1<<6)) return GES_CLOCKWISE;
    if (g0 & (1<<7)) return GES_ANTICLOCKWISE;
    if (g1 & (1<<0)) return GES_WAVE;
    else return GES_NONE;
    if (g0 != GES_NONE || g1 != GES_NONE ) paj_write(0x40, 0x01);

}

static HAL_StatusTypeDef paj_init_full(void) {
    HAL_Delay(1);
    paj_select_bank(0);

    uint8_t id0=0, id1=0;
    paj_read(0x00, &id0); // PartID low (0x20)
    paj_read(0x01, &id1); // PartID high (0x76)
    if(id0 != 0x20 || id1 != 0x76) {
    	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET);
    	return HAL_ERROR;
    }

    // Theo datasheet: sau power‑on, “wake” bằng truy cập I2C slave ID, sau đó ghi init & enable @Bank1:0x72=0x01. :contentReference[oaicite:10]{index=10}
    for (uint8_t i=0; i<sizeof(initRegisterArray)/sizeof(initRegisterArray[0]); i++) {
        if (paj_write(initRegisterArray[i][0], initRegisterArray[i][1]) != HAL_OK) {
        	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET);
        	HAL_Delay(1000);
        	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);
            return HAL_ERROR;
        }
        // nếu muốn chắc ăn, delay rất ngắn:
        // HAL_Delay(1);
    }
    paj_select_bank(0);
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
  if (paj_init_full() != HAL_OK) {
       HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
       HAL_Delay(1000);
       HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
       HAL_Delay(1000);
       HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);

  }
  paj_select_bank(1);
  paj_write(0x72, 0x01);
  paj_select_bank(0);
  while (1)
  {
	  gesture_t g = paj_read_gesture();
	  if (g != GES_NONE) {
		  CAN_DATA_TX[0]=(uint8_t)g;
		  HAL_CAN_AddTxMessage(&hcan, &CAN_pHeader, CAN_DATA_TX, &CAN_pTxMailbox);
		  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
		  HAL_Delay(3000);
	  }
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
	CAN_pHeader.StdId = 0x02;
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
    CAN_sFilterConfig.FilterIdHigh = 0x001 << 5;
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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
