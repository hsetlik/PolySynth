/*
 * LFO.h
 *
 *  Created on: Aug 31, 2024
 *      Author: hayden
 */

#ifndef INC_LFO_H_
#define INC_LFO_H_

#include "main.h"
#include "Patch.h"


#ifdef __cplusplus
#include "Perlin.h"

class LFOProcessor{
private:
	lfo_t* params;
	float phase;
	uint16_t lastCode;
	tick_t lastUpdateTick;
	float lastUpdateMs = 0.0f;
	PerlinGen perlin;
public:
	LFOProcessor();
	void setParams(lfo_t* lfo);
	void tick();
	void tickMs(float delta);
	uint16_t getCurrentValue();

};




#endif

#ifdef __cplusplus
#define EXTERNC extern "c"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_LFO_H_ */
