/*
 * ADSR.cpp
 *
 *  Created on: Aug 19, 2024
 *      Author: hayden
 */

#include "ADSR.h"
ADSRProcessor::ADSRProcessor() :
		params(nullptr), state(Idle), lastUpdateTick(0), msSinceStateChange(
				0.0f), level(0.0f), releaseStartLevel(0.0f) {

}

ADSRProcessor::ADSRProcessor(adsr_t *env) :
		params(env), state(Idle), lastUpdateTick(0), msSinceStateChange(0.0f), level(
				0.0f), releaseStartLevel(0.0f) {

}

void ADSRProcessor::gateOn() {
	if (!busy()){ // normal envelope starting from level = 0
		state = Attack;
		msSinceStateChange = 0.0f;
		lastUpdateTick = TickTimer_get();
	} else { // otherwise, deal with retriggering
		// find the point on the attack slope that matches our current level
		// ms / atttack = level
		state = Attack;
		msSinceStateChange = level * params->attack;
		lastUpdateTick = TickTimer_get();
	}
}

void ADSRProcessor::gateOff() {

	state = Release;
	msSinceStateChange = 0.0f;
	lastUpdateTick = TickTimer_get();
	releaseStartLevel = level;

}

uint16_t ADSRProcessor::nextDACCode() {

	tick_t now = TickTimer_get();
	msSinceStateChange += TickTimer_tickDistanceMs(lastUpdateTick, now);
	lastUpdateTick = now;
	switch (state) {
	case Idle:
		return 0;
	case Attack:
		if (msSinceStateChange > params->attack) {
			state = Decay;
			msSinceStateChange = 0.0f;
		} else {
			level = msSinceStateChange / params->attack;
		}
		break;
	case Decay:
		if (msSinceStateChange > params->decay) {
			state = Sustain;
			msSinceStateChange = 0.0f;
		} else {
			float diff = (1.0f - params->sustain)
					* (msSinceStateChange / params->decay);
			level = 1.0f - diff;
		}
		break;
	case Sustain:
		level = params->sustain;
		break;
	case Release:
		if (msSinceStateChange > params->release) {
			state = Idle;
			msSinceStateChange = 0.0f;
		} else {
			level = releaseStartLevel
					* (1.0f - (msSinceStateChange / params->release));
		}
		break;
	default:
		break;
	}
	// TODO: convert this to logarithmic for the VCA maybe?
	return (uint16_t) level * 4096.0f;

}
