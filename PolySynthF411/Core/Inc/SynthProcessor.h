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
#include "MIDI.h"
#include "ControlVoltage.h"

class SynthProcessor
{
private:
	VoiceClock* const voiceClock;
	//---STATE STUFF----
	uint8_t voicesInUse; //bits represent a voice in use
public:
	SynthProcessor(voice_clock_t vc);
	/**
	 * This needs to:
	 * - Calculate and update CVs when asked by main
	 * - Process MIDI events passed in by main
	 * - Process button/encoder inputs and update patch data
	 * - Send updates to the display and the neopixels as I see fit
	 */
	void updateDacLevels(dacLevels_t* levels);
	void processMidiMessage(midiMsg msg);
private:
	bool isVoiceActive(uint8_t voice);
	void startVoice(uint8_t voice);
	void endVoice(uint8_t voice);
	// returns index of the first free voice, or -1 if we're out of voices
	int8_t getFreeVoice();
};

#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/
typedef void* synth_proc_t;

EXTERNC synth_proc_t createSynthProc(voice_clock_t clk);
EXTERNC void updateDacLevels(synth_proc_t proc, dacLevels_t* levels);
EXTERNC void processMidiMessage(synth_proc_t proc, midiMsg msg);

#undef EXTERNC

#endif /* INC_SYNTHPROCESSOR_H_ */
