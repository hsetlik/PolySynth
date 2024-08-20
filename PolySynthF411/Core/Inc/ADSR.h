/*
 * ADSR.h
 *
 *  Created on: Aug 19, 2024
 *      Author: hayden
 */

#ifndef INC_ADSR_H_
#define INC_ADSR_H_

#include "main.h"
#include "Patch.h"

#ifdef __cplusplus


enum EnvState {
	Idle,
	Attack,
	Decay,
	Sustain,
	Release
};

class ADSRProcessor
{
private:
	adsr_t* const params;
	EnvState state;
	tick_t lastUpdateTick;
	float msSinceStateChange;
	float level;
	float releaseStartLevel;
public:
	ADSRProcessor(adsr_t* env);
	bool busy() {return state != Idle;}
	void gateOn();
	void gateOff();
	uint16_t nextDACCode();

};

#endif

#ifdef __cplusplus
#define EXTERNC extern "c"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_ADSR_H_ */
