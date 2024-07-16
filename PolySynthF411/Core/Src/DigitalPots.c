/*
 * DigitalPots.c
 *
 *  Created on: Jun 23, 2024
 *      Author: hayden
 */
#include "DigitalPots.h"

// this is based on p. 56 of the datasheet
// NOTE: the rightmost two bits are unused in our case
uint8_t MCP44x1_writeByte(uint8_t channel){
	switch(channel){
	case 0:
		return 0x00;
	case 1:
		return 0x10;
	case 2:
		return 0x60;
	case 3:
		return 0x70;
	default:
		return 0x00;
	}
}


void MCP44x1_setLevel(I2C_HandleTypeDef* i2c, uint8_t addr, uint8_t channel, uint8_t value){
	uint8_t data[2];
	data[0] = MCP44x1_writeByte(channel);
	data[1] = value;
	uint16_t addr16 = ((uint16_t)addr) << 8;
	if(HAL_I2C_Master_Transmit(i2c, addr16, data, 2, HAL_MAX_DELAY)!= HAL_OK){
		// uh oh
	}
}


//==================================================================
