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
#define NUM_MOD_DESTS 8
typedef enum {
	CUTOFF,
	RESONANCE,
	FOLD,
	PWM1,
	PWM2,
	TUNE1,
	TUNE2,
	VCA
}ModDest;

#define NUM_MOD_SOURCES 9

typedef enum {
	NONE,
	ENV1,
	ENV2,
	LFO1,
	LFO2,
	LFO3,
	MODWHL,
	PITCHWHL,
	VEL
}ModSource;

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
#define MAX_MODULATIONS 72 // equal to sources * dests

typedef uint16_t mod_t;

typedef struct {
	uint8_t sources[NUM_MOD_SOURCES];
	uint8_t numMods;
} mod_list_t;

typedef struct {
	mod_t mods[MAX_MODULATIONS];
} modmatrix_t; // takes 144 bytes to store all this

mod_t* get_mod(modmatrix_t* mat, uint8_t src, uint8_t dest);

mod_list_t get_mods_for_dest(modmatrix_t mat, uint8_t dest);

uint16_t apply_mod_offset(uint8_t dest, uint16_t baseDacCode, int16_t modOffset);


// helper functions for the data packing described above ^^
uint8_t get_mod_source(mod_t mod);
uint8_t get_mod_dest(mod_t mod);
int8_t get_mod_depth(mod_t mod);
void set_mod_depth(mod_t* m, int8_t d);

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

#define ENV_LENGTH_MAX ATTACK_MAX + DECAY_MAX + RELEASE_MAX
#define ENV_LENGTH_MIN ATTACK_MIN + DECAY_MIN + RELEASE_MIN

typedef struct{
	float attack;
	float decay;
	float sustain;
	float release;
} adsr_t; // these take 16 bytes each

// OSCILLATORS==============================================================
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

// LFO==============================================================
#define NUM_LFO_TYPES 4
typedef enum{
	Sine,
	Triangle,
	Ramp,
	Perlin
} LFOType;

#define LFO_FREQ_MIN 0.001f
#define LFO_FREQ_MAX 25.0f
#define LFO_FREQ_DEFAULT 1.5f

typedef struct{
float freq;
uint8_t lfoType;
} lfo_t; // 5 bytes each


// FILTER/FOLDER==============================================================
#define RES_MAX 255
#define RES_MIN 0
#define RES_DEFAULT 40

#define CUTOFF_MAX 4095
#define CUTOFF_MIN 0
#define CUTOFF_DEFAULT 3100

#define FOLD_MAX 4095
#define FOLD_MIN 0
#define FOLD_DEFAULT 0


typedef struct {
	// the below add up to 8 bytes VVV
	uint16_t cutoffBase;
	uint16_t resBase;
	uint16_t foldBase;
	uint8_t foldFirst;
	uint8_t highPassMode;

	modmatrix_t modMatrix; // 144 bytes

	dco_t oscs[2]; // 2 x 8 = 16 bytes
	adsr_t envs[2]; // 2 x 16 = 32 bytes

	lfo_t lfos[3]; // 3 x 5 = 15 bytes

	uint8_t useSustainPedal;

} patch_t;



#define PATCH_SIZE_BYTES 216

patch_t getDefaultPatch();

// enum to assign IDs to every base parameter (ie not including modulations or depths)
#define NUM_PARAMS 33

typedef enum {
	// envelopes
	pEnv1Attack,
	pEnv1Decay,
	pEnv1Sustain,
	pEnv1Release,

	pEnv2Attack,
	pEnv2Decay,
	pEnv2Sustain,
	pEnv2Release,
	// LFOs
	pLFO1Freq,
	pLFO1Mode,
	pLFO2Freq,
	pLFO2Mode,
	pLFO3Freq,
	pLFO3Mode,
	// Oscillators
	pOsc1Coarse,
	pOsc1Fine,
	pOsc1PulseWidth,
	pOsc1SquareLevel,
	pOsc1SawLevel,
	pOsc1TriLevel,
	pOsc1OscLevel,

	pOsc2Coarse,
	pOsc2Fine,
	pOsc2PulseWidth,
	pOsc2SquareLevel,
	pOsc2SawLevel,
	pOsc2TriLevel,
	pOsc2OscLevel,
	// filter
	pFilterCutoff,
	pFilterRes,
	pFilterMode,
	//folder
	pFoldLevel,
	pFoldFirst
} ParamID;


#ifdef __cplusplus
}
#endif

#endif /* INC_PATCH_H_ */
