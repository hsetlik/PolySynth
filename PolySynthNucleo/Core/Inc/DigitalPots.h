/*
 * DigitalPots.h
 *
 *  Created on: Jun 23, 2024
 *      Author: hayden
 */

#ifndef INC_DIGITALPOTS_H_
#define INC_DIGITALPOTS_H_
#ifdef __cplusplus
extern "C" {
#endif


#include "main.h"

//========================================================================
// Two MCP4451 quad digital pots handle mixing the oscillators
#define MCP44X1_1_ADDR 0x58
#define MCP44X1_2_ADDR 0x5C

// per section 7.4 of the datasheet writing to the pot register works like:
// - address byte (taken care of by the I2C HAL)
// - command byte (see p. 55 of the datasheet)
// - data byte

void MCP44x1_setLevel(I2C_HandleTypeDef* i2c, uint8_t addr, uint8_t channel, uint8_t value);


//========================================================================
// the filter resonance is controlled by a AD5280 digital pot
#define AD5280_ADDR 0x5A

void AD5280_setLevel(I2C_HandleTypeDef* i2c, uint8_t addr, uint8_t value);



#ifdef __cplusplus
}
#endif

#endif /* INC_DIGITALPOTS_H_ */
