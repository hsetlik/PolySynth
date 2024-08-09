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
	const uint8_t mask = 0x0F; // binary 00001111
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

//========================================================================

patch_t getDefaultPatch(){
	patch_t patch;

	patch.cutoffBase = CUTOFF_DEFAULT;
	patch.foldBase = FOLD_DEFAULT;
	patch.foldFirst = 0;
	patch.highPassMode = 0;

	patch.oscillators[0].coarseTune = 0;
	patch.oscillators[0].fineTune = 0;
	patch.oscillators[0].pulseWidth = PWM_DEFAULT;
	patch.oscillators[0].pulseLevel = 0;
	patch.oscillators[0].triLevel = 0;
	patch.oscillators[0].sawLevel = LEVEL_DEFAULT;
	patch.oscillators[0].oscLevel = LEVEL_DEFAULT;

	patch.oscillators[1].coarseTune = -12;
	patch.oscillators[1].fineTune = 0;
	patch.oscillators[1].pulseWidth = PWM_DEFAULT;
	patch.oscillators[1].pulseLevel = LEVEL_DEFAULT;
	patch.oscillators[1].triLevel = 0;
	patch.oscillators[1].sawLevel = 0;
	patch.oscillators[1].oscLevel = LEVEL_DEFAULT;

	patch.envelopes[0].attack = ATTACK_DEFAULT;
	patch.envelopes[0].decay = DECAY_DEFAULT;
	patch.envelopes[0].sustain = SUSTAIN_DEFAULT;
	patch.envelopes[0].release = RELEASE_DEFAULT;

	return patch;
}
