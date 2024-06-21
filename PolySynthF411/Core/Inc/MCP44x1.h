/*
 * MCP44x1.h
 *
 *  Created on: Jun 2, 2024
 *      Author: hayden
 *
 *  	Driver for Microchip's MCP44xx line of quad I2C digital pots
 *  	Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/MCP4432.pdf
 */

#ifndef INC_MCP44X1_H_
#define INC_MCP44X1_H_
#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx_hal.h"
// for two devices addressed differently:
#define MCP44X1_1_ADDR 0x5A
#define MCP44X1_2_ADDR 0x5E

// per section 7.4 of the datasheet writing to the pot register works like:
// - address byte (taken care of by the I2C HAL)
// - command byte (see p. 55 of the datasheet)
// - data byte

uint8_t getWriteByteFor(uint8_t channel);

void MCP44x1_setLevel(I2C_HandleTypeDef* i2c, uint8_t addr, uint8_t channel, uint8_t value);






#ifdef __cplusplus
}
#endif

#endif /* INC_MCP44X1_H_ */
