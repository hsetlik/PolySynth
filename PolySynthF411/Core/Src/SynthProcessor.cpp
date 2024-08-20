/*
 * SynthProcessor.cpp
 *
 *  Created on: Aug 9, 2024
 *      Author: hayden
 */
#include "SynthProcessor.h"

SynthProcessor::SynthProcessor(voice_clock_t vc) :
		voiceClock(static_cast<VoiceClock*>(vc)), currentPatch(
				getDefaultPatch()), voicesInUse(0), sustainPedalDown(false) {
	// give all the envelopes the correct pointer to the patch data
	for (uint8_t v = 0; v < 6; v++) {
		env1Voices[v].setParams(&currentPatch.envelopes[0]);
		env2Voices[v].setParams(&currentPatch.envelopes[1]);
	}

}

void SynthProcessor::updateDacLevels(dacLevels_t *levels) {
	// remember, the argument pointer is an array of dacLevels arranged PER VOICE
	for (uint8_t v = 0; v < 6; v++) {
		if (isVoiceActive(v)) {
			// TODO: grip each of the 7 DAC voltages
			levels->currentData[DACChannel::VCA_CH] = env1Voices[v].nextDACCode();
			levels->currentData[DACChannel::AC1_CH] = ampComp1[v];
			levels->currentData[DACChannel::AC2_CH] = ampComp2[v];
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

//==================================================================================
synth_proc_t createSynthProc(voice_clock_t clk) {
	return new SynthProcessor(clk);
}

void updateDacLevels(synth_proc_t proc, dacLevels_t *levels) {
	SynthProcessor *ptr = static_cast<SynthProcessor*>(proc);
	ptr->updateDacLevels(levels);
}

void processMidiMessage(synth_proc_t proc, midiMsg msg) {
	SynthProcessor *ptr = static_cast<SynthProcessor*>(proc);
	ptr->processMidiMessage(msg);
}

