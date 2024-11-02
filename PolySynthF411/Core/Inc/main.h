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
#include "TickTimer.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
// this is used to access our display queue class
typedef void* display_queue_t;

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

extern void StatusLED_off(uint8_t led);
extern void StatusLED_on(uint8_t led);
extern void StatusLED_slowBlink(uint8_t led);
extern void StatusLED_fastBlink(uint8_t led);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define HP_MODE_Pin GPIO_PIN_1
#define HP_MODE_GPIO_Port GPIOC
#define FOLD_FIRST_Pin GPIO_PIN_2
#define FOLD_FIRST_GPIO_Port GPIOC
#define BATT_REF_Pin GPIO_PIN_0
#define BATT_REF_GPIO_Port GPIOA
#define PIXELS_MAIN_Pin GPIO_PIN_1
#define PIXELS_MAIN_GPIO_Port GPIOA
#define PIXELS_MATRIX_Pin GPIO_PIN_2
#define PIXELS_MATRIX_GPIO_Port GPIOA
#define PITCH_RST_Pin GPIO_PIN_4
#define PITCH_RST_GPIO_Port GPIOA
#define PITCH_SCK_Pin GPIO_PIN_5
#define PITCH_SCK_GPIO_Port GPIOA
#define PITCH_MOSI_Pin GPIO_PIN_7
#define PITCH_MOSI_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_4
#define LED1_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_5
#define LED2_GPIO_Port GPIOC
#define DAC_INH_Pin GPIO_PIN_0
#define DAC_INH_GPIO_Port GPIOB
#define DAC_A0_Pin GPIO_PIN_1
#define DAC_A0_GPIO_Port GPIOB
#define DAC_A1_Pin GPIO_PIN_2
#define DAC_A1_GPIO_Port GPIOB
#define DAC_A2_Pin GPIO_PIN_10
#define DAC_A2_GPIO_Port GPIOB
#define EXP_CS_Pin GPIO_PIN_12
#define EXP_CS_GPIO_Port GPIOB
#define EXP_SCK_Pin GPIO_PIN_13
#define EXP_SCK_GPIO_Port GPIOB
#define EXP_MISO_Pin GPIO_PIN_14
#define EXP_MISO_GPIO_Port GPIOB
#define EXP_MOSI_Pin GPIO_PIN_15
#define EXP_MOSI_GPIO_Port GPIOB
#define EXP_INTR_A_Pin GPIO_PIN_6
#define EXP_INTR_A_GPIO_Port GPIOC
#define EXP_INTR_A_EXTI_IRQn EXTI9_5_IRQn
#define EXP_INTR_B_Pin GPIO_PIN_7
#define EXP_INTR_B_GPIO_Port GPIOC
#define EXP_INTR_B_EXTI_IRQn EXTI9_5_IRQn
#define EXP_INTR_C_Pin GPIO_PIN_8
#define EXP_INTR_C_GPIO_Port GPIOC
#define EXP_INTR_C_EXTI_IRQn EXTI9_5_IRQn
#define DAC_SDA_Pin GPIO_PIN_9
#define DAC_SDA_GPIO_Port GPIOC
#define DAC_SCL_Pin GPIO_PIN_8
#define DAC_SCL_GPIO_Port GPIOA
#define LDAC_Pin GPIO_PIN_9
#define LDAC_GPIO_Port GPIOA
#define DISP_CS_Pin GPIO_PIN_15
#define DISP_CS_GPIO_Port GPIOA
#define DISP_SCK_Pin GPIO_PIN_10
#define DISP_SCK_GPIO_Port GPIOC
#define DISP_MISO_Pin GPIO_PIN_11
#define DISP_MISO_GPIO_Port GPIOC
#define DISP_MOSI_Pin GPIO_PIN_12
#define DISP_MOSI_GPIO_Port GPIOC
#define SD_CS_Pin GPIO_PIN_2
#define SD_CS_GPIO_Port GPIOD
#define DISP_DC_Pin GPIO_PIN_4
#define DISP_DC_GPIO_Port GPIOB
#define DISP_RST_Pin GPIO_PIN_5
#define DISP_RST_GPIO_Port GPIOB
#define MIDI_TX_Pin GPIO_PIN_6
#define MIDI_TX_GPIO_Port GPIOB
#define MIDI_RX_Pin GPIO_PIN_7
#define MIDI_RX_GPIO_Port GPIOB
#define POT_SCL_Pin GPIO_PIN_8
#define POT_SCL_GPIO_Port GPIOB
#define POT_SDA_Pin GPIO_PIN_9
#define POT_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define NUM_VOICES 6
// the master clock used by APB2 peripherals (including our voice clocks)
#define APB2_CLK 48000000 // 48MHz


/* Comment out `#define FLOAT_VOICE_MODE` below
 * to use the old integer code. Float mode
 * is more CPU intensive but should mean
 * far less tuning error at lower SPI clock
 * frequencies. (as of 10/15 it's at 3MhZ)
 */

#define FLOAT_VOICE_MODE

/* This should save us a bunch of float math in the
 * DAC/ CV updating code, comment it out if
 * something is weird
 */
#define DAC_UPDATE_OPTIMIZE

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;
extern display_queue_t mainDispQueue;
extern uint8_t SD_INITIALIZED;

// PWM/timer stuff
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim5;
extern DMA_HandleTypeDef hdma_tim2_ch2_ch4;
extern DMA_HandleTypeDef hdma_tim5_ch3_up;

#define SD_SPI_HANDLE hspi3

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
