/*
 * DAC7578.c
 *
 *  Created on: May 31, 2024
 *      Author: hayden
 */
#include "DAC7578.h"


// as per Table 3. in the datasheet
uint8_t DAC7578_getAddress(uint8_t addr0Hi){
	if(addr0Hi)
		return 0x4A;
	return 0x48;

}

// see table 6. in the datasheet
uint8_t DAC7578_getWriteCmdByte(uint8_t dac){
	return 0x30 | dac;
}



void DAC7578_setLevel(I2C_HandleTypeDef* i2c, uint8_t addr0hi, uint8_t dac, uint16_t value){
	uint16_t addr16 = (uint16_t)DAC7578_getAddress(addr0hi) << 8;
	uint8_t data[3];
	// command byte
	data[0] = DAC7578_getWriteCmdByte(dac);
	// data bytes
	value = value << 4;
	data[1] = (uint8_t)value;
	data[2] = (uint8_t)(value >> 8);

	if(HAL_I2C_Master_Transmit(i2c, addr16, data, 3, HAL_MAX_DELAY)!= HAL_OK){
		// uh oh
	}
}
