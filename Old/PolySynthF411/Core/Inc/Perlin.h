/*
 * Perlin.h
 *
 *  Created on: Sep 1, 2024
 *      Author: hayden
 */

#ifndef INC_PERLIN_H_
#define INC_PERLIN_H_
#include "main.h"

#ifdef __cplusplus

class PerlinGen {
private:
	float xPos = 0.0f;
	uint8_t octaves = 4;
	float lastFreq = 1.0f;
	float lacunarity = 2.0f;
	tick_t lastUpdateTick = 0;
public:
	PerlinGen();
	uint16_t getNextValue(float hz);

};

#endif

#ifdef __cplusplus
#define EXTERNC extern "c"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_PERLIN_H_ */
