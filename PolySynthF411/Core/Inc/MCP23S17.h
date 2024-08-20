/*
 * MCP23S17.h
 *
 *  Created on: Aug 20, 2024
 *      Author: hayden
 */

#ifndef INC_MCP23S17_H_
#define INC_MCP23S17_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "main.h"

/** Stuff to access the MCP23S17 SPI port expander
 *  Based on the datasheet here: https://ww1.microchip.com/downloads/en/DeviceDoc/20001952C.pdf

 */

#define EXP_1_ADDR 0x48 // mod matrix buttons
#define EXP_2_ADDR 0x4E // right side of control panel
#define EXP_3_ADDR 0x4C // left side of control panel

// Register addresses as described on p. 17 of the datasheet
// NOTE: these addresses assume we're using IOCON.BANK = 0 as
// is the default value

#define MCP23S17_IODIRA 0x00
#define MCP23S17_IODIRB 0x01
#define MCP23S17_IPOLA 0x02
#define MCP23S17_IPOLB 0x03
#define MCP23S17_GPINTENA 0x04
#define MCP23S17_GPINTENB 0x05
#define MCP23S17_DEFVALA 0x06
#define MCP23S17_DEFVALB 0x07
#define MCP23S17_INTCONA 0x08
#define MCP23S17_INTCONB 0x09
#define MCP23S17_IOCON 0x0A
#define MCP23S17_GPPUA 0x0C
#define MCP23S17_GPPUB 0x0D
#define MCP23S17_INTFA 0x0E
#define MCP23S17_INTFB 0x0F
#define MCP23S17_INTCAPA 0x10
#define MCP23S17_INTCAPB 0x11
#define MCP23S17_GPIOA 0x12
#define MCP23S17_GPIOB 0x13
#define MCP23S17_OLATA 0x14
#define MCP23S17_OLATB 0x15



int8_t MCP32S17_init(SPI_HandleTypeDef* spi, uint8_t addr);

void MCP23S17_setInterruptOnPin(SPI_HandleTypeDef* spi, uint8_t addr, uint8_t port, uint8_t pin, uint8_t activeLow);


#ifdef __cplusplus
}
#endif

#endif /* INC_MCP23S17_H_ */
