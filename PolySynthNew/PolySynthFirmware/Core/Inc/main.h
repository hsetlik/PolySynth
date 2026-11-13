/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

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
#define MIDI_RX_Pin GPIO_PIN_2
#define MIDI_RX_GPIO_Port GPIOE
#define MIDI_TX_Pin GPIO_PIN_3
#define MIDI_TX_GPIO_Port GPIOE
#define UI_TX_Pin GPIO_PIN_0
#define UI_TX_GPIO_Port GPIOA
#define UI_RX_Pin GPIO_PIN_1
#define UI_RX_GPIO_Port GPIOA
#define UI_BOOT0_Pin GPIO_PIN_2
#define UI_BOOT0_GPIO_Port GPIOA
#define UI_NRST_Pin GPIO_PIN_3
#define UI_NRST_GPIO_Port GPIOA
#define DAC_CS_Pin GPIO_PIN_4
#define DAC_CS_GPIO_Port GPIOA
#define DAC_SCK_Pin GPIO_PIN_5
#define DAC_SCK_GPIO_Port GPIOA
#define DAC_MOSI_Pin GPIO_PIN_7
#define DAC_MOSI_GPIO_Port GPIOA
#define DAC_CLR_Pin GPIO_PIN_4
#define DAC_CLR_GPIO_Port GPIOC
#define COUNTER_RST_Pin GPIO_PIN_5
#define COUNTER_RST_GPIO_Port GPIOC
#define PITCH_RST_Pin GPIO_PIN_12
#define PITCH_RST_GPIO_Port GPIOB
#define PITCH_SCK_Pin GPIO_PIN_13
#define PITCH_SCK_GPIO_Port GPIOB
#define PITCH_MOSI_Pin GPIO_PIN_15
#define PITCH_MOSI_GPIO_Port GPIOB
#define SD_INSERT_Pin GPIO_PIN_6
#define SD_INSERT_GPIO_Port GPIOC
#define SD_PWR_Pin GPIO_PIN_7
#define SD_PWR_GPIO_Port GPIOC
#define POTS_RST_Pin GPIO_PIN_4
#define POTS_RST_GPIO_Port GPIOB
#define POTS_SCL_Pin GPIO_PIN_6
#define POTS_SCL_GPIO_Port GPIOB
#define POTS_SDA_Pin GPIO_PIN_7
#define POTS_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
