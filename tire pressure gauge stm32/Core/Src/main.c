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
 * Desenvolvido por Caio Dantas Costa para uso no Projeto Integrador III
 * Faculdade de Tecnologia de Jundiaí - Deputado Ary Fossen
 * 2025
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include "ssd1306_conf.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "smp3011.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAL_SAMP		30
#define PA_PSI_X10		68947
#define PSI_BAR_X100		6894
#define SCALE			1000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
int32_t pressure_offset = 0, current_pressure_cal, avg_pressure,
		avg_pressure_bar;
int row;
char display_buffer[116];
bool isCalibrated = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
//#define PRINTF2UART2 int __io_putchar(int data)
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void ssd1306_Write(uint8_t col, uint8_t row, char *texto, bool limpar_tela,
bool atualizar_tela) {
	if (limpar_tela)
		ssd1306_Fill(Black);
	ssd1306_SetCursor(col, row);
	ssd1306_WriteString(texto, Font_7x10, White);
	if (atualizar_tela)
		ssd1306_UpdateScreen();
}

void smp3011_calibrate(int samples) {
	for (int num = 0; num < samples; num++) {
		smp3011_read();
		pressure_offset += smp3011_get_pressure();
		HAL_Delay(100);
	}
	pressure_offset /= CAL_SAMP;

	isCalibrated = true;

}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

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
	MX_I2C1_Init();
	/* USER CODE BEGIN 2 */
	ssd1306_Init();
	smp3011_init(&hi2c1);

	smp3011_calibrate(CAL_SAMP);
	ssd1306_Write(0, 0, "Calibrando...", true, true);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		row = 0;
		smp3011_read();
		if (isCalibrated == true) {

			current_pressure_cal = smp3011_get_pressure() - pressure_offset; // Isso (talvez) deve ajustar o diferencial do sensor para zero.
			if (current_pressure_cal > 0) {	
				for (int i = 0; i < 5; i++) {
					current_pressure_cal += current_pressure_cal;
					HAL_Delay(200);
				}
				avg_pressure = ((current_pressure_cal * SCALE) / PA_PSI_X10) / 5;
				avg_pressure_bar = (avg_pressure * PSI_BAR_X100) / 10000;
			} else {
				avg_pressure = 0;
			}
		}
		// Exibição da pressão no display
		snprintf((char*) display_buffer, sizeof(display_buffer),
				"%ld.%01ld psi", avg_pressure / 10, avg_pressure % 10);
		ssd1306_Write(0, row += 16, display_buffer, false, false);

		snprintf((char*) display_buffer, sizeof(display_buffer),
				"%ld.%01ld bar", avg_pressure_bar / 100, avg_pressure_bar % 100);
		ssd1306_Write(0, row += 20, display_buffer, false, false);

		snprintf((char*) display_buffer, sizeof(display_buffer),
				"%ld Pa avg", current_pressure_cal / 5);
		ssd1306_Write(0, row += 28, display_buffer, false, false);

		// Exibição de avisos no display
		if (0 == avg_pressure) {
			ssd1306_Write(0, 0, "Inicie...", false, false);

		} else if (30 > avg_pressure) {
			ssd1306_Write(0, 0, "Pressao baixa!	", false, false);

		} else if (35 >= avg_pressure && avg_pressure >= 30) {
			ssd1306_Write(0, 0, "Pressao OK!	", false, false);

		} else {
			ssd1306_Write(0, 0, "Pressao alta!	", false, false);

		}
		HAL_Delay(100);
		/* USER CODE END WHILE */
		
		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	__HAL_FLASH_SET_LATENCY(FLASH_LATENCY_1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x10805D88;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
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
static void MX_GPIO_Init(void) {
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
