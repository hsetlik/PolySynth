/*
 * SynthProcessor.h
 *
 *  Created on: Aug 9, 2024
 *      Author: hayden
 */

#ifndef INC_SYNTHPROCESSOR_H_
#define INC_SYNTHPROCESSOR_H_



#ifdef __cplusplus
#include "Patch.h"
#include "VoiceClock.h"

class SynthProcessor
{
	VoiceClock* const voiceClock;
public:
	SynthProcessor(voice_clock_t vc);
};

#endif

#ifdef __cplusplus
#define EXTERNC extern "c"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_SYNTHPROCESSOR_H_ */
