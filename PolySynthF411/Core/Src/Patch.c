/*
 * Patch.c
 *
 *  Created on: Jul 25, 2024
 *      Author: hayden
 */
#include "Patch.h"

uint8_t getModSource(mod_t mod){
	uint8_t msb = (uint8_t)(mod >> 8);
	return (msb >> 4);
}

uint8_t getModDest(mod_t mod) {
	uint8_t msb = (uint8_t)(mod >> 8);
	const uint8_t mask = 0x0F; // equivalent to binary 00001111
	return msb & mask;
}

int8_t getModDepth(mod_t mod) {
	const uint16_t mask = 255;
	uint16_t m = mod & mask;
	return (int8_t)m;
}

mod_t createMod(uint8_t source, uint8_t dest, int8_t depth){
	uint16_t msb = (uint16_t)(dest | (source << 4));
	mod_t data = msb << 8;
	return data | depth;
}


