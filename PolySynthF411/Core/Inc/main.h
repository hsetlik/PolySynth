/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32f4xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PITCH_CS_Pin GPIO_PIN_4
#define PITCH_CS_GPIO_Port GPIOA
#define PITCH_SCK_Pin GPIO_PIN_5
#define PITCH_SCK_GPIO_Port GPIOA
#define PITCH_MOSI_Pin GPIO_PIN_7
#define PITCH_MOSI_GPIO_Port GPIOA
#define EXP_CS_Pin GPIO_PIN_12
#define EXP_CS_GPIO_Port GPIOB
#define EXP_SCK_Pin GPIO_PIN_13
#define EXP_SCK_GPIO_Port GPIOB
#define EXP_MISO_Pin GPIO_PIN_14
#define EXP_MISO_GPIO_Port GPIOB
#define EXP_MOSI_Pin GPIO_PIN_15
#define EXP_MOSI_GPIO_Port GPIOB
#define EXP_INTR_Pin GPIO_PIN_6
#define EXP_INTR_GPIO_Port GPIOC
#define LDAC_Pin GPIO_PIN_8
#define LDAC_GPIO_Port GPIOC
#define DAC_SDA_Pin GPIO_PIN_9
#define DAC_SDA_GPIO_Port GPIOC
#define DAC_SCL_Pin GPIO_PIN_8
#define DAC_SCL_GPIO_Port GPIOA
#define DAC_A0_Pin GPIO_PIN_9
#define DAC_A0_GPIO_Port GPIOA
#define DAC_A1_Pin GPIO_PIN_10
#define DAC_A1_GPIO_Port GPIOA
#define DAC_A2_Pin GPIO_PIN_11
#define DAC_A2_GPIO_Port GPIOA
#define DISP_CS_Pin GPIO_PIN_15
#define DISP_CS_GPIO_Port GPIOA
#define DISP_SCK_Pin GPIO_PIN_10
#define DISP_SCK_GPIO_Port GPIOC
#define DISP_MISO_Pin GPIO_PIN_11
#define DISP_MISO_GPIO_Port GPIOC
#define DISP_MOSI_Pin GPIO_PIN_12
#define DISP_MOSI_GPIO_Port GPIOC
#define POT_SCL_Pin GPIO_PIN_6
#define POT_SCL_GPIO_Port GPIOB
#define POT_SDA_Pin GPIO_PIN_7
#define POT_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
