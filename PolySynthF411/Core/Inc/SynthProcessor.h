/*
 * SynthProcessor.h
 *
 *  Created on: Aug 9, 2024
 *      Author: hayden
 */

#ifndef INC_SYNTHPROCESSOR_H_
#define INC_SYNTHPROCESSOR_H_



#include "Patch.h"
#include "VoiceClock.h"
#include "MIDI.h"
#include "ControlVoltage.h"
#include "ADSR.h"
#include "Button.h"
#include "Encoder.h"
#include "Graphics.h"

#ifdef __cplusplus

class SynthProcessor
{
private:
	VoiceClock* const voiceClock;
	EncoderProcessor* const encoderProc;
	ButtonProcessor* const buttonProc;
	patch_t currentPatch;
	//---STATE STUFF----
	uint8_t voicesInUse; //bits represent a voice in use
	uint8_t voiceVelocity[6];
	uint8_t voiceNotes[6];
	uint16_t ampComp1[6];
	uint16_t ampComp2[6];
	ADSRProcessor env1Voices[6];
	ADSRProcessor env2Voices[6];

	// control change stuff
	bool sustainPedalDown;
	uint16_t pitchWhlPos;
	uint16_t modWhlPos;

public:
	SynthProcessor(voice_clock_t vc, enc_processor_t ep, button_processor_t bp);

	/**
	 * This needs to:
	 * - Calculate and update CVs when asked by main
	 * - Process MIDI events passed in by main
	 * - Update patch data based on button/encoder events
	 * - Send updates to the display and the neopixels as I see fit
	 */
	void updateDacLevels(dacLevels_t* levels);
	void processMidiMessage(midiMsg msg);
	//Control callbacks
	// for encoder callbacks
	void handleEncoderTurn(uint8_t id, uint8_t clockwise);
	// for button callbacks
	void handleOnClick(uint8_t button);
	void handleOnPressStart(uint8_t button);
	void handleOnPressEnd(uint8_t button);
	void handleDuringPress(uint8_t button);
private:
	bool isVoiceActive(uint8_t voice);
	void startVoice(uint8_t voice);
	void endVoice(uint8_t voice);
	// returns index of the first free voice, or -1 if we're out of voices
	int8_t getFreeVoice();
	int8_t getVoiceForNote(uint8_t note);
	void startNote(uint8_t note, uint8_t vel);
	void endNote(uint8_t note);
	// helper for calculating our mod matrix stuff
	uint16_t modDestValue(uint8_t dest, uint8_t voice);
	uint16_t modSourceValue(uint8_t src, uint8_t voice);
	int16_t modSourceOffset(uint16_t src, uint8_t dest, uint8_t voice);

	uint16_t velocityValue12Bit(uint8_t voice);



};

#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/
typedef void* synth_processor_t;

EXTERNC synth_processor_t create_synth_processor(voice_clock_t clk, enc_processor_t ep, button_processor_t bp);
EXTERNC void update_dac_levels(synth_processor_t proc, dacLevels_t* levels);
EXTERNC void process_midi_msg(synth_processor_t proc, midiMsg msg);
// for setting up function pointers in main.c
EXTERNC void handle_on_click(synth_processor_t synth, uint8_t button);
EXTERNC void handle_on_press_start(synth_processor_t synth, uint8_t button);
EXTERNC void handle_on_press_end(synth_processor_t synth, uint8_t button);
EXTERNC void handle_during_press(synth_processor_t synth, uint8_t button);

EXTERNC void handle_encoder_turn(synth_processor_t synth, uint8_t enc, uint8_t dir);

#undef EXTERNC

#endif /* INC_SYNTHPROCESSOR_H_ */
