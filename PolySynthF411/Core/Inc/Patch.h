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
	ENV1,
	ENV2,
	LFO1,
	LFO2,
	LFO3,
	MODWHL,

};

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
} adsr_t;

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

} dco_t;











#ifdef __cplusplus
}
#endif

#endif /* INC_PATCH_H_ */
