/*
 * Patch.c
 *
 *  Created on: Jul 25, 2024
 *      Author: hayden
 */
#include "Patch.h"

uint8_t get_mod_source(mod_t mod){
	uint8_t msb = (uint8_t)(mod >> 8);
	return (msb >> 4);
}

uint8_t get_mod_dest(mod_t mod) {
	uint8_t msb = (uint8_t)(mod >> 8);
	const uint8_t mask = 0x0F; // binary 00001111
	return msb & mask;
}

int8_t get_mod_depth(mod_t mod) {
	const uint16_t mask = 255;
	uint16_t m = mod & mask;
	return (int8_t)m;
}

mod_t createMod(uint8_t source, uint8_t dest, int8_t depth){
	uint16_t msb = (uint16_t)(dest | (source << 4));
	mod_t data = msb << 8;
	return data | depth;
}


mod_t get_mod(modmatrix_t mat, uint8_t src, uint8_t dest){
	return mat.mods[(src * dest) + dest];
}


mod_list_t get_mods_for_dest(modmatrix_t mat, uint8_t dest){
	mod_list_t list;
	list.numMods = 0;
	mod_t current;
	for(uint8_t src = 0; src < NUM_MOD_SOURCES; src++){
		current = get_mod(mat, src, dest);
		if(current){
			list.sources[list.numMods] = src;
			++list.numMods;
		}
	}
	return list;
}

// just a little helper to clean up apply_mod_offset
uint16_t clampModRange(uint16_t max, uint16_t min, uint16_t baseValue, int16_t offset){
	uint16_t fullVal = (uint16_t)((int16_t)baseValue + offset);
	if(fullVal < min)
		return min;
	if(fullVal > max)
		return max;
	return fullVal;
}

int16_t centsTo12Bit(int8_t tune){
	float fVal = (float)tune / 100.0f;
	return (int16_t)fVal * 4095.0f;
}


uint16_t apply_mod_offset(uint8_t dest, uint16_t baseDacCode, int16_t modOffset){
	ModDest id = (ModDest)dest;
	switch (id) {
	case CUTOFF:
		return clampModRange(CUTOFF_MAX, CUTOFF_MIN, baseDacCode, modOffset);
	case RESONANCE:
		return clampModRange(RES_MAX, RES_MIN, baseDacCode, modOffset);
	case FOLD:
		return clampModRange(FOLD_MAX, FOLD_MIN, baseDacCode, modOffset);
	case PWM1:
		return clampModRange(PWM_MAX, PWM_MIN, baseDacCode, modOffset);
	case PWM2:
		return clampModRange(PWM_MAX, PWM_MIN, baseDacCode, modOffset);
	case TUNE1: // TODO: figure out how deal with these three
		break;
	case TUNE2:
		break;
	case VCA:
		break;
	default:
		break;
	}
	return 0;

}

//========================================================================

patch_t getDefaultPatch(){
	patch_t patch;

	patch.cutoffBase = CUTOFF_DEFAULT;
	patch.resBase = RES_DEFAULT;
	patch.foldBase = FOLD_DEFAULT;
	patch.foldFirst = 0;
	patch.highPassMode = 0;
	patch.useSustainPedal = 1;

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
