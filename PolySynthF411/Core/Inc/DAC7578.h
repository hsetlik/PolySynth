/*
 * DAC7578.h
 *
 *  Created on: May 31, 2024
 *      Author: hayden
 *      HAL-based driver for TI's DAC7578 octal I2C DAC
 *      Code all based on datasheet
 */

#ifndef INC_DAC7578_H_
#define INC_DAC7578_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "main.h"

#define ADDR_0_HI 0


void DAC7578_setLevel(I2C_HandleTypeDef* i2c, uint8_t dac, uint16_t value);


#ifdef __cplusplus
}
#endif
#endif /* INC_DAC7578_H_ */
