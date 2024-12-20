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
#include "LFO.h"
#include "Button.h"
#include "Encoder.h"
#include "Graphics/Graphics.h"
#include "NeopixelChain.h"
#include "PatchFile.h"

#ifdef __cplusplus

class SynthProcessor
{
private:
	VoiceClock* const voiceClock;
	EncoderProcessor* const encoderProc;
	ButtonProcessor* const buttonProc;
	GraphicsProcessor* const graphicsProc;
	PixelProcessor* const pixelProc;
	PatchBrowser pb;
	patch_t patch;
	//---STATE STUFF----
	uint8_t voicesInUse; //bits represent a voice in use
	uint8_t voiceVelocity[NUM_VOICES];
	uint8_t voiceNotes[NUM_VOICES];
	uint16_t ampComp1[NUM_VOICES];
	uint16_t ampComp2[NUM_VOICES];
	ADSRProcessor env1Voices[NUM_VOICES];
	ADSRProcessor env2Voices[NUM_VOICES];
	LFOProcessor lfos[3];

	// control change stuff
	bool sustainPedalDown = false;
	bool portamentoOn = false;
	// pitch bend expressed as a float between -1 and 1
	float currentPitchBend = 0.0f;
	uint16_t modWhlPos;
	uint8_t expressionLevel = 0;

	// bitfield tokeep track of notes to switch off when the sustain
	// pedal is lifted
	uint8_t sustainedNotes = 0;

	// device state stuff
	bool inAltMode = false;
	uint8_t visibleView = ViewID::vEnv1;
	uint8_t selectedPWM = ParamID::pOsc1PulseWidth;

	// mod matrix state stuff
	mod_t* selectedMod = nullptr;
	bool inBank2 = false;
	uint8_t lastDestBtn = 0;
	uint8_t lastSrcBtn = 0;
	int8_t currentModPrevDepth = 0;
	tick_t lastSrcClickAt = 0;
	tick_t lastDestClickAt = 0;
	void processSrcClick(uint8_t btn);
	void processDestClick(uint8_t btn);
	void handleModClick(uint8_t srcID, uint8_t destID);
	// update the graphics processor as needed

public:
	SynthProcessor(voice_clock_t vc, enc_processor_t ep, button_processor_t bp, graphics_processor_t gp, pixel_processor_t pp);

	/**
	 * This needs to:
	 * - Calculate and update CVs when asked by main
	 * - Process MIDI events passed in by main
	 * - Update patch data based on button/encoder events
	 * - Send updates to the display and the neopixels as I see fit
	 */
#ifndef DAC_UPDATE_OPTIMIZE
	void updateDacLevels(dacLevels_t* levels);
#else
	void updateDacLevels(dacLevels_t* levels, float deltaMs);
#endif

	void processMidiMessage(midi_t msg);
	//Control callbacks
	// for encoder callbacks
	void handleEncoderTurn(uint8_t id, uint8_t clockwise);
	// for button callbacks
	void handleOnClick(uint8_t button);
	void handleOnPressStart(uint8_t button);
	void handleOnPressEnd(uint8_t button);
	void handleDuringPress(uint8_t button);
	// main callback for keeping the display up to date
	void checkGUIUpdates();
	// call this to init the SD card
	void initFileSystem();

private:
	bool alt() {
		return inAltMode;
	}
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
	int16_t modSourceValue(uint8_t src, uint8_t voice);
	int16_t modSourceOffset(uint16_t src, uint8_t dest, uint8_t voice);

	// convert the 7-bit MIDI velocity value to a 12 bit DAC-friendly value
	uint16_t velocityValue12Bit(uint8_t voice);
	// handle MIDI cc
	void handleControlChange(uint8_t controller, uint8_t data);
	void handleSustainPedalEnd();

	//CONTROL STUFF==============================
	void nudgeParameter(uint8_t id, bool dir);
	void nudgeModDepth(mod_t* mod, bool dir);
	//  nudge the appropriate parameter for this encoder given the selected view
	void handleViewEncoder(uint8_t enc, bool dir);

};

#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/
typedef void* synth_processor_t;

EXTERNC synth_processor_t create_synth_processor(voice_clock_t clk, enc_processor_t ep, button_processor_t bp, graphics_processor_t gp, pixel_processor_t pp);

#ifndef DAC_UPDATE_OPTIMIZE
EXTERNC void update_dac_levels(synth_processor_t proc, dacLevels_t* levels);
#else
EXTERNC void update_dac_levels(synth_processor_t proc, dacLevels_t* levels, float delta);
#endif
EXTERNC void process_midi_msg(synth_processor_t proc, midi_t msg);
// for setting up function pointers in main.c
EXTERNC void handle_on_click(synth_processor_t synth, uint8_t button);
EXTERNC void handle_on_press_start(synth_processor_t synth, uint8_t button);
EXTERNC void handle_on_press_end(synth_processor_t synth, uint8_t button);
EXTERNC void handle_during_press(synth_processor_t synth, uint8_t button);

EXTERNC void handle_encoder_turn(synth_processor_t synth, uint8_t enc, uint8_t dir);

EXTERNC void check_gui_updates(synth_processor_t proc);
EXTERNC void init_file_system(synth_processor_t proc);

#undef EXTERNC

#endif /* INC_SYNTHPROCESSOR_H_ */
