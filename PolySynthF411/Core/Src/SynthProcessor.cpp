/*
 * SynthProcessor.cpp
 *
 *  Created on: Aug 9, 2024
 *      Author: hayden
 */
#include "SynthProcessor.h"

SynthProcessor::SynthProcessor(voice_clock_t vc, enc_processor_t ep,
		button_processor_t bp) :
		voiceClock(static_cast<VoiceClock*>(vc)), encoderProc(
				static_cast<EncoderProcessor*>(ep)), buttonProc(
				static_cast<ButtonProcessor*>(bp)), currentPatch(
				getDefaultPatch()), voicesInUse(0), sustainPedalDown(false), pitchWhlPos(
				0), modWhlPos(0) {
	// give all the envelopes the correct pointer to the patch data
	for (uint8_t v = 0; v < 6; v++) {
		env1Voices[v].setParams(&currentPatch.envelopes[0]);
		env2Voices[v].setParams(&currentPatch.envelopes[1]);
	}
	// do the same for the LFOs
	for (uint8_t l = 0; l < 3; l++) {
		lfos[l].setParams(&currentPatch.lfos[l]);
	}

}

void SynthProcessor::updateDacLevels(dacLevels_t *levels) {
	//tick our LFOs
	for (uint8_t i = 0; i < 3; i++) {
		lfos[i].tick();
	}
	for (uint8_t v = 0; v < 6; v++) {
		if (isVoiceActive(v)) {
	// REMEMBER the argument pointer is an array of dacLevels arranged PER VOICE
			levels[v].currentData[DACChannel::VCA_CH] = env1Voices[v].nextDACCode();
			levels[v].currentData[DACChannel::AC1_CH] = ampComp1[v];
			levels[v].currentData[DACChannel::AC2_CH] = ampComp2[v];
			levels[v].currentData[DACChannel::CUTOFF_CH] = modDestValue((uint8_t) ModDest::CUTOFF, v);
			levels[v].currentData[DACChannel::FOLD_CH] = modDestValue((uint8_t) ModDest::FOLD, v);
			levels[v].currentData[DACChannel::PWM1_CH] = modDestValue((uint8_t) ModDest::PWM1, v);
			levels[v].currentData[DACChannel::PWM2_CH] = modDestValue((uint8_t) ModDest::PWM2, v);
		}
	}

}

void SynthProcessor::processMidiMessage(midiMsg msg) {
	MIDIMsgType msgType = (MIDIMsgType) msg.msgType;
	switch (msgType) {
	case NoteOn:
		startNote(msg.data[0], msg.data[1]);
		break;
	case NoteOff:
		endNote(msg.data[0]);
		break;
	case ControlChange:
		break;
	case PitchBend:
		break;
	default:
		break;
	}
}
//======================================================================
bool SynthProcessor::isVoiceActive(uint8_t voice) {
	return voicesInUse & (1 << voice);
}

void SynthProcessor::startVoice(uint8_t voice) {
	voicesInUse = voicesInUse | (1 << voice);
}

void SynthProcessor::endVoice(uint8_t voice) {
	voicesInUse = voicesInUse & ~(1 << voice);
}

int8_t SynthProcessor::getFreeVoice() {
	for (int8_t v = 0; v < 6; v++) {
		if (!isVoiceActive(v))
			return v;
	}
	return -1;
}

int8_t SynthProcessor::getVoiceForNote(uint8_t note) {

	// first check if we're already playing the note
	for (uint8_t v = 0; v < 6; v++) {
		if (isVoiceActive(v) && voiceNotes[v] == note)
			return (int8_t) v;
	}
	return getFreeVoice();
}

void SynthProcessor::startNote(uint8_t note, uint8_t vel) {
	int8_t v = getVoiceForNote(note);
	if (v != -1) {
		// 1. set our state
		startVoice(v);
		voiceNotes[v] = note;
		voiceVelocity[v] = vel;
		// consult patch data for tuning of each osc
		//TODO: we need to calculate how to offset these fine tuning values
		// for pitch modulation
		float hz1 = hzForTuning(note, currentPatch.oscillators[0].coarseTune,
				currentPatch.oscillators[0].fineTune);
		float hz2 = hzForTuning(note, currentPatch.oscillators[1].coarseTune,
				currentPatch.oscillators[1].fineTune);
		ampComp1[v] = dacValueForHz(hz1);
		ampComp2[v] = dacValueForHz(hz2);
		// set the VoiceClock object to the resulting value in hz
		voiceClock->setFrequency(2 * v, hz1);
		voiceClock->setFrequency((2 * v) + 1, hz2);

		// start envelopes
		env1Voices[v].gateOn();
		env2Voices[v].gateOn();
	}
}

void SynthProcessor::endNote(uint8_t note) {
	for (uint8_t v = 0; v < 6; v++) {
		if (isVoiceActive(v) && voiceNotes[v] == note) {
			env1Voices[v].gateOff();
			env2Voices[v].gateOff();
			return;
		}
	}
}
//MOD MATRIX ==================================================================================

uint16_t SynthProcessor::modDestValue(uint8_t dest, uint8_t voice) {
	mod_list_t mods = get_mods_for_dest(currentPatch.modMatrix, dest);
	int16_t offset = 0;
	for (uint8_t m = 0; m < mods.numMods; m++) {
		offset += modSourceOffset(mods.sources[m], dest, voice);
	}
	ModDest id = (ModDest) dest;
	switch (id) {
	case CUTOFF:
		return apply_mod_offset(dest, currentPatch.cutoffBase, offset);
	case RESONANCE:
		return apply_mod_offset(dest, currentPatch.resBase, offset);
	case FOLD:
		return apply_mod_offset(dest, currentPatch.foldBase, offset);
	case PWM1:
		return apply_mod_offset(dest, currentPatch.oscillators[0].pulseWidth,
				offset);
	case PWM2:
		return apply_mod_offset(dest, currentPatch.oscillators[1].pulseWidth,
				offset);
	case TUNE1:
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

uint16_t SynthProcessor::velocityValue12Bit(uint8_t voice) {
	float fVel = (float) voiceVelocity[voice] / 127.0f;
	return (uint16_t) fVel * 4096.0f;
}

uint16_t SynthProcessor::modSourceValue(uint8_t src, uint8_t voice) {
	ModSource id = (ModSource) src;
	switch (id) {
	case ENV1:
		return env1Voices[voice].prevDACCode();
	case ENV2:
		return env1Voices[voice].prevDACCode();
		break;
	case LFO1:
		return lfos[0].getCurrentValue();
	case LFO2:
		return lfos[1].getCurrentValue();
	case LFO3:
		return lfos[2].getCurrentValue();
	case MODWHL:
		return modWhlPos;
	case PITCHWHL:
		return pitchWhlPos;
	case VEL:
		return velocityValue12Bit(voice);
	default:
		break;

	}
	return 0;
}

int16_t SynthProcessor::modSourceOffset(uint16_t src, uint8_t dest,
		uint8_t voice) {
	mod_t mod = get_mod(currentPatch.modMatrix, src, dest);
	float val = (float) modSourceValue(src, voice);
	return (int16_t) (val * ((float) get_mod_depth(mod) / 127.0f));
}

//CONTROLS==================================================================================

// Encoders---------------
void SynthProcessor::handleEncoderTurn(uint8_t num, uint8_t clockwise) {
	EncID id = (EncID) num;
	switch (id) {
	case A:
		break;
	case B:
		break;
	case C:
		break;
	case D:
		break;
	case MenuEnc:
		break;
	case Depth:
		break;
	case Cutoff:
		break;
	case Res:
		break;
	case PWM:
		break;
	case Fold:
		break;
	default:
		break;
	}
}

// Buttons---------------
void SynthProcessor::handleOnClick(uint8_t button) {
	ButtonID id = (ButtonID) button;
	switch (id) {
	case Alt:
		break;
	case Env1:
		break;
	case Env2:
		break;
	case LFO1:
		break;
	case LFO2:
		break;
	case LFO3:
		break;
	case Osc1:
		break;
	case Osc2:
		break;
	case PWMB:
		break;
	case FilterMode:
		break;
	case FoldFirst:
		break;
	case Menu:
		break;
	case MS1:
		break;
	case MS2:
		break;
	case MS3:
		break;
	case MS4:
		break;
	case MS5:
		break;
	case MS6:
		break;
	case ModBank:
		break;
	case MD1:
		break;
	case MD2:
		break;
	case MD3:
		break;
	case MD4:
		break;
	case MD5:
		break;
	case MD6:
		break;
	case MD7:
		break;
	case MD8:
		break;
	default:
		break;
	}
}

void SynthProcessor::handleOnPressStart(uint8_t button) {
	ButtonID id = (ButtonID) button;
	switch (id) {
	case Alt:
		break;
	case Env1:
		break;
	case Env2:
		break;
	case LFO1:
		break;
	case LFO2:
		break;
	case LFO3:
		break;
	case Osc1:
		break;
	case Osc2:
		break;
	case PWMB:
		break;
	case FilterMode:
		break;
	case FoldFirst:
		break;
	case Menu:
		break;
	case MS1:
		break;
	case MS2:
		break;
	case MS3:
		break;
	case MS4:
		break;
	case MS5:
		break;
	case MS6:
		break;
	case ModBank:
		break;
	case MD1:
		break;
	case MD2:
		break;
	case MD3:
		break;
	case MD4:
		break;
	case MD5:
		break;
	case MD6:
		break;
	case MD7:
		break;
	case MD8:
		break;
	default:
		break;
	}
}

void SynthProcessor::handleOnPressEnd(uint8_t button) {
	ButtonID id = (ButtonID) button;
	switch (id) {
	case Alt:
		break;
	case Env1:
		break;
	case Env2:
		break;
	case LFO1:
		break;
	case LFO2:
		break;
	case LFO3:
		break;
	case Osc1:
		break;
	case Osc2:
		break;
	case PWMB:
		break;
	case FilterMode:
		break;
	case FoldFirst:
		break;
	case Menu:
		break;
	case MS1:
		break;
	case MS2:
		break;
	case MS3:
		break;
	case MS4:
		break;
	case MS5:
		break;
	case MS6:
		break;
	case ModBank:
		break;
	case MD1:
		break;
	case MD2:
		break;
	case MD3:
		break;
	case MD4:
		break;
	case MD5:
		break;
	case MD6:
		break;
	case MD7:
		break;
	case MD8:
		break;
	default:
		break;
	}
}

void SynthProcessor::handleDuringPress(uint8_t button) {
	ButtonID id = (ButtonID) button;
	switch (id) {
	case Alt:
		break;
	case Env1:
		break;
	case Env2:
		break;
	case LFO1:
		break;
	case LFO2:
		break;
	case LFO3:
		break;
	case Osc1:
		break;
	case Osc2:
		break;
	case PWMB:
		break;
	case FilterMode:
		break;
	case FoldFirst:
		break;
	case Menu:
		break;
	case MS1:
		break;
	case MS2:
		break;
	case MS3:
		break;
	case MS4:
		break;
	case MS5:
		break;
	case MS6:
		break;
	case ModBank:
		break;
	case MD1:
		break;
	case MD2:
		break;
	case MD3:
		break;
	case MD4:
		break;
	case MD5:
		break;
	case MD6:
		break;
	case MD7:
		break;
	case MD8:
		break;
	default:
		break;
	}
}

//==================================================================================
synth_processor_t create_synth_processor(voice_clock_t clk, enc_processor_t ep,
		button_processor_t bp) {
	return new SynthProcessor(clk, ep, bp);
}

void update_dac_levels(synth_processor_t proc, dacLevels_t *levels) {
	SynthProcessor *ptr = static_cast<SynthProcessor*>(proc);
	ptr->updateDacLevels(levels);
}

void process_midi_msg(synth_processor_t proc, midiMsg msg) {
	SynthProcessor *ptr = static_cast<SynthProcessor*>(proc);
	ptr->processMidiMessage(msg);
}

void handle_on_click(synth_processor_t synth, uint8_t button) {

	SynthProcessor *ptr = static_cast<SynthProcessor*>(synth);
	ptr->handleOnClick(button);
}

void handle_on_press_start(synth_processor_t synth, uint8_t button) {

	SynthProcessor *ptr = static_cast<SynthProcessor*>(synth);
	ptr->handleOnPressStart(button);
}

void handle_on_press_end(synth_processor_t synth, uint8_t button) {

	SynthProcessor *ptr = static_cast<SynthProcessor*>(synth);
	ptr->handleOnPressEnd(button);
}

void handle_during_press(synth_processor_t synth, uint8_t button) {

	SynthProcessor *ptr = static_cast<SynthProcessor*>(synth);
	ptr->handleDuringPress(button);
}
//-----------------------
void handle_encoder_turn(synth_processor_t synth, uint8_t enc, uint8_t dir) {
	SynthProcessor *ptr = static_cast<SynthProcessor*>(synth);
	ptr->handleEncoderTurn(enc, dir);
}
