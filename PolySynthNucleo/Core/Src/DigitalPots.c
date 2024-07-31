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
		Error_Handler();
	}
}


//==================================================================

// all this bit stuff is explained on pg 16 of the AD5280 datasheet
uint8_t AD5280_addressByte(uint8_t addr, uint8_t read) {
	if(read)
		return addr | 1;
	return addr & ~1;
}

// some helpers for bit logic stuff
/*
 * Per the datasheet the bits (MSB to LSB order) for the command byte are:
 * 1. RDAC: RDAC select bit (should always be low for AD5280)
 * 2. RS: Midscale reset. Logic high will set the wiper to its midpoint
 * 3. SD: Shutdown. Logic high shorts the wiper to terminal B
 * 4. O1: o1 logic output
 * 5. O2: o2 logic output
 * 6. Don't care
 * 7. Don't care
 * 8. Don't care
 *
 * (basically we can just send 0x00 in our case woo)
 * */


void AD5280_setLevel(I2C_HandleTypeDef* i2c, uint8_t addr, uint8_t value) {
	uint16_t addr16 = ((uint16_t)AD5280_addressByte(addr, 0)) << 8;
	uint8_t data[2];
	data[0] = 0x00;
	data[1] = value;
	if(HAL_I2C_Master_Transmit(i2c, addr16, data, 2, HAL_MAX_DELAY)!= HAL_OK){
		Error_Handler();
	}
}
