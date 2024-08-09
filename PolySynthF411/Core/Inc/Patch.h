/*
 * Patch.h
 *
 *  Created on: Jul 25, 2024
 *      Author: hayden
 */

#ifndef INC_PATCH_H_
#define INC_PATCH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

//==============================================================================================================
// modulation matrix storage/logic
enum ModDestination{
	CUTOFF,
	RESONANCE,
	FOLD,
	PWM1,
	PWM2,
	TUNE1,
	TUNE2,
	VCA
};

enum ModSource{
	NONE,
	ENV1,
	ENV2,
	LFO1,
	LFO2,
	LFO3,
	MODWHL,
	PITCHWHL,
	VEL
};

/**
 * We have eight sources (not counting NONE) and eight destinations,
 * which means we need to be able to store a maximum of 64 modulations.
 * Each modulation can be represented in two bytes, which works like:
 *
 * MOST SIGNUFiCANT BYTE:
 * Since we only have 8-9 possible values for the source and destination,
 * we can represent each in four bits, where the most significant four bits are
 * the source index and the least significant four bits are the destination ID
 *
 * LEAST SIGNIFICANT BYTE:
 * Just an 8-bit SIGNED integer to represent depth
 *
 */
#define MAX_MODULATIONS 64

typedef uint16_t mod_t;

typedef struct {
	mod_t mods[MAX_MODULATIONS];
	uint8_t modsInUse;
} modmatrix_t; // takes 129 bytes to store all this

// helper functions for the data packing described above ^^
uint8_t getModSource(mod_t mod);
uint8_t getModDest(mod_t mod);
int8_t getModDepth(mod_t mod);

mod_t createMod(uint8_t source, uint8_t dest, int8_t depth);

//============================================================================
// params for our ADSR envelopes. these are float values in milliseconds
#define ATTACK_MIN 2.5f
#define ATTACK_MAX 250.0f
#define ATTACK_DEFAULT 20.0f

#define DECAY_MIN 5.0f
#define DECAY_MAX 600.0f
#define DECAY_DEFAULT 100.0f

#define SUSTAIN_MIN	0.0f
#define SUSTAIN_MAX 1.0f
#define SUSTAIN_DEFAULT 0.6f

#define RELEASE_MIN 3.0f
#define RELEASE_MAX 4000.0f
#define RELEASE_DEFAULT 200.0f

typedef struct{
	float attack;
	float decay;
	float sustain;
	float release;
} adsr_t; // these take 16 bytes each

// params for the DCOs
#define COARSE_MIN -24
#define COARSE_MAX 24

#define FINE_MIN -100
#define FINE_MAX 100

#define PWM_MIN 30
#define PWM_MAX 4085
#define PWM_DEFAULT 2047

#define LEVEL_DEFAULT 120

typedef struct{
	int8_t coarseTune;
	int8_t fineTune;

	uint16_t pulseWidth;

	uint8_t pulseLevel;
	uint8_t sawLevel;
	uint8_t triLevel;
	uint8_t oscLevel;

} dco_t; // 8 bytes each

//TODO: LFO state and default values and stuff should eventually go here

#define CUTOFF_MAX 4096
#define CUTOFF_MIN 0
#define CUTOFF_DEFAULT 3100

#define FOLD_MAX 4096
#define FOLD_MIN 0
#define FOLD_DEFAULT 0

#define PATCH_SIZE_BYTES 183 //NOTE: this number will change when we add LFOs and stuff

typedef struct {
	uint16_t cutoffBase;
	uint16_t foldBase;
	uint8_t foldFirst;
	uint8_t highPassMode;

	modmatrix_t modMatrix;

	dco_t oscillators[2];
	adsr_t envelopes[2];

} patch_t;

patch_t getDefaultPatch();
//TODO: functions to encode/decode the patch struct in bytes here


#ifdef __cplusplus
}
#endif

#endif /* INC_PATCH_H_ */
