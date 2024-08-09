/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OE_Pin GPIO_PIN_13
#define OE_GPIO_Port GPIOC
#define CHARG_Pin GPIO_PIN_1
#define CHARG_GPIO_Port GPIOA
#define CHARG_EXTI_IRQn EXTI1_IRQn
#define C0_Pin GPIO_PIN_2
#define C0_GPIO_Port GPIOA
#define C1_Pin GPIO_PIN_3
#define C1_GPIO_Port GPIOA
#define C2_Pin GPIO_PIN_4
#define C2_GPIO_Port GPIOA
#define C3_Pin GPIO_PIN_5
#define C3_GPIO_Port GPIOA
#define C4_Pin GPIO_PIN_6
#define C4_GPIO_Port GPIOA
#define C5_Pin GPIO_PIN_7
#define C5_GPIO_Port GPIOA
#define DATA_R_Pin GPIO_PIN_0
#define DATA_R_GPIO_Port GPIOB
#define DATA_G_Pin GPIO_PIN_1
#define DATA_G_GPIO_Port GPIOB
#define DATA_B_Pin GPIO_PIN_2
#define DATA_B_GPIO_Port GPIOB
#define LOAD_Pin GPIO_PIN_10
#define LOAD_GPIO_Port GPIOB
#define CLK_Pin GPIO_PIN_11
#define CLK_GPIO_Port GPIOB
#define C12_Pin GPIO_PIN_12
#define C12_GPIO_Port GPIOB
#define C13_Pin GPIO_PIN_13
#define C13_GPIO_Port GPIOB
#define C14_Pin GPIO_PIN_14
#define C14_GPIO_Port GPIOB
#define C15_Pin GPIO_PIN_15
#define C15_GPIO_Port GPIOB
#define C6_Pin GPIO_PIN_8
#define C6_GPIO_Port GPIOA
#define C7_Pin GPIO_PIN_9
#define C7_GPIO_Port GPIOA
#define C8_Pin GPIO_PIN_10
#define C8_GPIO_Port GPIOA
#define C9_Pin GPIO_PIN_3
#define C9_GPIO_Port GPIOB
#define C10_Pin GPIO_PIN_4
#define C10_GPIO_Port GPIOB
#define C11_Pin GPIO_PIN_5
#define C11_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
