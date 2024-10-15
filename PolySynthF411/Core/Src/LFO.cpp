/*
 * LFO.cpp
 *
 *  Created on: Aug 31, 2024
 *      Author: hayden
 */
#include "LFO.h"
#include <cmath>
//============================================================

float incrementPhase(float input, float hz, float deltaMs) {
	float periodMs = 1000.0f / hz;
	return std::fmod(input + (deltaMs / periodMs), 1.0f);
}

uint16_t sineAtPhase(float phase) {
	static const float twoPi = 2.0f * std::acos(-1.0f);
	float fNorm = ((std::sin(phase * twoPi) + 1.0f) / 2.0f);
	return (uint16_t) fNorm * 4096.0f;
}

uint16_t triangleAtPhase(float phase) {
	if (phase < 0.5f) {
		return (uint16_t) phase * 8192.0f;
	} else {
		float fDrop = (phase - 0.5f) * 2.0f;
		return (uint16_t) 4096.0f * (1.0f - fDrop);
	}
}

uint16_t rampAtPhase(float phase) {
	return (uint16_t) phase * 4096.0f;
}

//============================================================

LFOProcessor::LFOProcessor() :
		params(nullptr), phase(0.0f), lastCode(0), lastUpdateTick(0) {

}

void LFOProcessor::setParams(lfo_t *lfo) {
	params = lfo;
}

void LFOProcessor::tick() {
	tick_t now = TickTimer_get();
	float deltaMs = TickTimer_tickDistanceMs(lastUpdateTick, now);
	lastUpdateTick = now;
	phase = incrementPhase(phase, params->freq, deltaMs);
	switch ((LFOType) params->lfoType) {
	case Sine:
		lastCode = sineAtPhase(phase);
		break;
	case Triangle:
		lastCode = triangleAtPhase(phase);
		break;
	case Ramp:
		lastCode = rampAtPhase(phase);
		break;
	case Perlin:
		lastCode = perlin.getNextValue(params->freq);
		break;
	default:
		break;
	}
}

void LFOProcessor::tickMs(float delta) {
	phase = incrementPhase(phase, params->freq, delta);
	switch ((LFOType) params->lfoType) {
	case Sine:
		lastCode = sineAtPhase(phase);
		break;
	case Triangle:
		lastCode = triangleAtPhase(phase);
		break;
	case Ramp:
		lastCode = rampAtPhase(phase);
		break;
	case Perlin:
		lastCode = perlin.getNextValue(params->freq);
		break;
	default:
		break;
	}
}

uint16_t LFOProcessor::getCurrentValue() {
	return lastCode;
}
