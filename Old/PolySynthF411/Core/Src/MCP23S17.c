/*
 * MCP23S17.c
 *
 *  Created on: Aug 20, 2024
 *      Author: hayden
 */
#include "MCP23S17.h"

// some helpers just for this file

// We shouldn't need this anymore bc the CS pin is hardware-defined now (I think)
//void setCSPin(uint8_t high) {
//	if (high)
//		HAL_GPIO_WritePin(EXP_CS_GPIO_Port, EXP_CS_Pin, GPIO_PIN_SET);
//	else
//		HAL_GPIO_WritePin(EXP_CS_GPIO_Port, EXP_CS_Pin, GPIO_PIN_RESET);
//}

uint8_t readRegister(SPI_HandleTypeDef *spi, uint8_t reg, uint8_t addr) {
	uint8_t dest[2];
	uint8_t msg[2];
	msg[0] = addr | 0x01; // set the read bit
	msg[1] = reg;
	//setCSPin(0);
	if (HAL_SPI_TransmitReceive(spi, msg, dest, 2, 100) != HAL_OK) {
		Error_Handler();
	}
	// do stuff
	//setCSPin(1);
	return dest[0];
}

int8_t writeRegisterI(SPI_HandleTypeDef *spi, uint8_t addr, uint8_t reg,
		uint8_t data) {
	uint8_t msg[3];
	msg[0] = addr;
	msg[1] = reg;
	msg[2] = data;
	//setCSPin(0);
	if (HAL_SPI_Transmit(spi, msg, 2, 100) != HAL_OK) {
		Error_Handler();
		return -1;
	}
	//setCSPin(1);
	return 1;

}

void writeRegister(SPI_HandleTypeDef *spi, uint8_t addr, uint8_t reg,
		uint8_t data) {
	uint8_t msg[3];
	msg[0] = addr;
	msg[1] = reg;
	msg[2] = data;
	//setCSPin(0);
	if (HAL_SPI_Transmit(spi, msg, 3, 100) != HAL_OK) {
		Error_Handler();
	}
	//setCSPin(1);

}

void setPinDefval(SPI_HandleTypeDef *spi, uint8_t addr, uint8_t port,
		uint8_t pin, uint8_t val) {
	if (port) { // we're on port B
		uint8_t defVal = readRegister(spi, MCP23S17_DEFVALB, addr);
		if (val)
			defVal = defVal | (1 << pin);
		else
			defVal = defVal & ~(1 << pin);
		writeRegister(spi, addr, MCP23S17_DEFVALB, defVal);
	} else { // we're on port A
		uint8_t defVal = readRegister(spi, MCP23S17_DEFVALA, addr);
		if (val)
			defVal = defVal | (1 << pin);
		else
			defVal = defVal & ~(1 << pin);
		writeRegister(spi, addr, MCP23S17_DEFVALA, defVal);
	}
}
//====================================================================================

int8_t MCP32S17_init(SPI_HandleTypeDef *spi, uint8_t addr) {
	// 1. set the IOCON byte to use SPI addressing mode
	uint8_t iocon = readRegister(spi, MCP23S17_IOCON, addr);
	iocon = iocon | (1 << 3);
	if (writeRegisterI(spi, addr, MCP23S17_IOCON, iocon) < 0) {
		return -1;
	}
	return 1;
}

void MCP23S17_setInterruptOnPin(SPI_HandleTypeDef *spi, uint8_t addr,
		uint8_t port, uint8_t pin, uint8_t activeLow) {
	//1: make sure the defVal is set before we modify the interrupt registers
	setPinDefval(spi, addr, port, pin, activeLow);
	// 2: grab the register values so we don't remove existing interrupts
	uint8_t gpinten = 0;
	if (port)
		gpinten = readRegister(spi, MCP23S17_GPINTENB, addr);
	else
		gpinten = readRegister(spi, MCP23S17_GPINTENA, addr);
	// 3: set the appropriate bit
	gpinten = gpinten | (1 << pin);
	// 4: write back to the same register
	if (port)
		writeRegister(spi, addr, MCP23S17_GPINTENB, gpinten);
	else
		writeRegister(spi, addr, MCP23S17_GPINTENA, gpinten);

	// 5: do a similar thing for the INTCON register

	// this sets it such that the interrupt generator compares to the
	// defval rather than the pin's previous level.
	// in practice this should mean that we only generate interrupts on falling edges
	uint8_t intcon = 0;
	if (port)
		intcon = readRegister(spi, MCP23S17_INTCONB, addr);
	else
		intcon = readRegister(spi, MCP23S17_INTCONA, addr);
	intcon = intcon | (1 << pin);
	if (port)
		writeRegister(spi, addr, MCP23S17_INTCONB, intcon);
	else
		writeRegister(spi, addr, MCP23S17_INTCONA, intcon);
}


void MCP23S17_setInputOnPin(SPI_HandleTypeDef* spi, uint8_t addr, uint8_t port, uint8_t pin, uint8_t activeLow){
	setPinDefval(spi, addr, port, pin, activeLow);
}


uint8_t MCP23S17_getGPIOBits(SPI_HandleTypeDef* spi, uint8_t addr, uint8_t port){
	if(port)
		return readRegister(spi, MCP23S17_GPIOB, addr);
	return readRegister(spi, MCP23S17_GPIOA, addr);
}


uint8_t MCP23S17_getLatchBits(SPI_HandleTypeDef* spi, uint8_t addr, uint8_t port){
	if(port)
		return readRegister(spi, MCP23S17_OLATB, addr);
	return readRegister(spi, MCP23S17_OLATA, addr);
}


uint8_t MCP23S17_getLastInterruptPin(SPI_HandleTypeDef* spi, uint8_t addr, uint8_t port){
	uint8_t intf = 0;
	if(port){
		intf = readRegister(spi, MCP23S17_INTFB, addr);
		// I *think* we need to clear this register before the next interrupt?
		writeRegister(spi, addr, MCP23S17_INTFB, 0x00);
	}
	else {
		intf = readRegister(spi, MCP23S17_INTFA, addr);
		writeRegister(spi, addr, MCP23S17_INTFA, 0x00);
	}
	for(uint8_t i = 0; i < 8; i ++){
		if(intf & (1 << i))
			return i;
	}
	return 0;
}
