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
	Idle, Attack, Decay, Sustain, Release
};

class ADSRProcessor {
private:
	adsr_t *params;
	EnvState state;
	tick_t lastUpdateTick;
	float msSinceStateChange;
	float level;
	float releaseStartLevel;
	uint16_t lastCode;
public:
	ADSRProcessor(adsr_t *env);
	ADSRProcessor();
	void setParams(adsr_t *p) {
		params = p;
	}
	bool busy() {
		return state != Idle;
	}
	void gateOn();
	void gateOff();
	void tick();
	void tickMs(float delta);
	uint16_t prevDACCode() {return lastCode;}

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
