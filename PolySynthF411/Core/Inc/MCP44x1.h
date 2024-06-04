/*
 * MCP44x1.h
 *
 *  Created on: Jun 2, 2024
 *      Author: hayden
 *
 *  	Driver for Microchip's MCP44x1 line of quad I2C digital pots
 *  	Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/MCP4432.pdf
 */

#ifndef INC_MCP44X1_H_
#define INC_MCP44X1_H_

// for two devices addressed differently:
#define MCP4X11_1_ADDR 0x5A
#define MCP4X11_2_ADDR 0x5E

// per section 7.4 of the datasheet writing to the pot register works like:
// - address byte (taken care of by the I2C HAL)
// - command byte (see the datasheet)
// - data byte



#endif /* INC_MCP44X1_H_ */
