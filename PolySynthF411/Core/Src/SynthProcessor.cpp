/*
 * SynthProcessor.cpp
 *
 *  Created on: Aug 9, 2024
 *      Author: hayden
 */
#include "SynthProcessor.h"

SynthProcessor::SynthProcessor(voice_clock_t vc, enc_processor_t ep,
		button_processor_t bp, graphics_processor_t gp, pixel_processor_t pp) :
		voiceClock(static_cast<VoiceClock*>(vc)), encoderProc(
				static_cast<EncoderProcessor*>(ep)), buttonProc(
				static_cast<ButtonProcessor*>(bp)), graphicsProc(
				static_cast<GraphicsProcessor*>(gp)), pixelProc(
				static_cast<PixelProcessor*>(pp)), patch(getDefaultPatch()), voicesInUse(
				0), sustainPedalDown(false), pitchWhlPos(0), modWhlPos(0) {
	// give all the envelopes the correct pointer to the patch data
	for (uint8_t v = 0; v < NUM_VOICES; v++) {
		env1Voices[v].setParams(&patch.envs[0]);
		env2Voices[v].setParams(&patch.envs[1]);
	}
	// do the same for the LFOs
	for (uint8_t l = 0; l < 3; l++) {
		lfos[l].setParams(&patch.lfos[l]);
	}

	// set up the graphics processor
	graphicsProc->setPatchData(&patch);
	graphicsProc->initViews();

}

void SynthProcessor::updateDacLevels(dacLevels_t *levels) {
	//tick our LFOs
	for (uint8_t i = 0; i < 3; i++) {
		lfos[i].tick();
	}
	for (uint8_t v = 0; v < NUM_VOICES; v++) {
		if (isVoiceActive(v)) {
			env1Voices[v].tick();
			env2Voices[v].tick();
			// REMEMBER the argument pointer is an array of dacLevels arranged PER VOICE
			levels[v].currentData[DACChannel::VCA_CH] = modDestValue(
					(uint8_t) ModDest::VCA, v);
			levels[v].currentData[DACChannel::AC1_CH] = ampComp1[v];
			levels[v].currentData[DACChannel::AC2_CH] = ampComp2[v];
			levels[v].currentData[DACChannel::CUTOFF_CH] = modDestValue(
					(uint8_t) ModDest::CUTOFF, v);
			levels[v].currentData[DACChannel::FOLD_CH] = modDestValue(
					(uint8_t) ModDest::FOLD, v);
			levels[v].currentData[DACChannel::PWM1_CH] = modDestValue(
					(uint8_t) ModDest::PWM1, v);
			levels[v].currentData[DACChannel::PWM2_CH] = modDestValue(
					(uint8_t) ModDest::PWM2, v);
		}
	}

}

void SynthProcessor::processMidiMessage(midi_t msg) {
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
	for (int8_t v = 0; v < NUM_VOICES; v++) {
		if (!isVoiceActive(v))
			return v;
	}
	return -1;
}

int8_t SynthProcessor::getVoiceForNote(uint8_t note) {

	// first check if we're already playing the note
	for (uint8_t v = 0; v < NUM_VOICES; v++) {
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
		float hz1 = hzForTuning(note, patch.oscs[0].coarseTune,
				patch.oscs[0].fineTune);
		float hz2 = hzForTuning(note, patch.oscs[1].coarseTune,
				patch.oscs[1].fineTune);
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
	for (uint8_t v = 0; v < NUM_VOICES; v++) {
		if (isVoiceActive(v) && voiceNotes[v] == note) {
			env1Voices[v].gateOff();
			env2Voices[v].gateOff();
			return;
		}
	}
}
//MOD MATRIX ==================================================================================

uint16_t SynthProcessor::modDestValue(uint8_t dest, uint8_t voice) {
	mod_list_t mods = get_mods_for_dest(patch.modMatrix, dest);
	int16_t offset = 0;
	for (uint8_t m = 0; m < mods.numMods; m++) {
		offset += modSourceOffset(mods.sources[m], dest, voice);
	}
	ModDest id = (ModDest) dest;
	switch (id) {
	case CUTOFF:
		return apply_mod_offset(dest, patch.cutoffBase, offset);
	case RESONANCE:
		return apply_mod_offset(dest, patch.resBase, offset);
	case FOLD:
		return apply_mod_offset(dest, patch.foldBase, offset);
	case PWM1:
		return apply_mod_offset(dest, patch.oscs[0].pulseWidth, offset);
	case PWM2:
		return apply_mod_offset(dest, patch.oscs[1].pulseWidth, offset);
	case TUNE1:
		break;
	case TUNE2:
		break;
	case VCA:
		return apply_mod_offset(dest, env1Voices[voice].prevDACCode(), offset);
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
	mod_t mod = *get_mod(&patch.modMatrix, src, dest);
	float val = (float) modSourceValue(src, voice);
	return (int16_t) (val * ((float) get_mod_depth(mod) / 127.0f));
}


void SynthProcessor::handleModClick(uint8_t src, uint8_t dest){
	mod_t* m = get_mod(&patch.modMatrix, src, dest);
	if(m != selectedMod){ // we've selected a new modulation
		selectedMod = m;
		currentModPrevDepth = 0;
	} else { // we've clicked on the current mod, so toggle it
		int8_t newDepth = get_mod_depth(*selectedMod);
		if(newDepth != 0){ // toggle it off
			currentModPrevDepth = newDepth;
			set_mod_depth(m, 0);
		} else { // toggle it on with the either the previous depth or some default 'on' depth
			int8_t onDepth = (currentModPrevDepth != 0) ? currentModPrevDepth : 40;
			set_mod_depth(m, onDepth);
		}
	}
}

//helper to tell if we've received a valid mod selection
bool isValidClick(tick_t srcTime, tick_t destTime){
	constexpr float maxMs = 4000.0f;
	tick_t first = std::min(srcTime, destTime);
	tick_t second = std::max(srcTime, destTime);
	return TickTimer_tickDistanceMs(first, second) < maxMs;
}


void SynthProcessor::processSrcClick(uint8_t btn){
	lastSrcClickAt = TickTimer_get();
	lastSrcBtn = btn;
	if(isValidClick(lastSrcClickAt, lastDestClickAt)){
		uint8_t srcID = inBank2 ? lastSrcBtn + 6 : lastSrcBtn;
		handleModClick(srcID, lastDestBtn);
	}
}

void SynthProcessor::processDestClick(uint8_t btn){
	lastDestClickAt = TickTimer_get();
	lastDestBtn = btn;
	if(isValidClick(lastSrcClickAt, lastDestClickAt)){
		uint8_t srcID = inBank2 ? lastSrcBtn + 6 : lastSrcBtn;
		handleModClick(srcID, lastDestBtn);
	}
}

void SynthProcessor::initFileSystem(){
	if(pb.init()){
		pb.loadAvailablePatches();
	}
}
//GUI------------

void SynthProcessor::checkGUIUpdates() {
	if (graphicsProc->needsLFOData()) {
		graphicsProc->updateLFOs(lfos[0].getCurrentValue(),
				lfos[1].getCurrentValue(), lfos[2].getCurrentValue());
	}
	graphicsProc->checkGUIUpdates();
}

//CONTROLS==================================================================================

/** This needs to
 * 1. update the currentPatch data as appropriate
 * 2. tell the GraphicsProcesor to react
 */

// helpers to nudge each type of parameter
// envs -----
float nudgeEnvAttack(float val, bool dir) {
	constexpr float atkNudge = 0.06f;
	if (dir)
		return std::min<float>(val * (1.0f + atkNudge), ATTACK_MAX);
	return std::max<float>(val * (1.0f - atkNudge), ATTACK_MIN);
}

float nudgeEnvDecay(float val, bool dir) {
	constexpr float decayNudge = 0.06f;
	if (dir)
		return std::min<float>(val * (1.0f + decayNudge), DECAY_MAX);
	return std::max<float>(val * (1.0f - decayNudge), DECAY_MIN);
}

float nudgeEnvSustain(float val, bool dir) {
	constexpr float sustainNudge = 0.08f;
	if (dir)
		return std::min<float>(val * (1.0f + sustainNudge), 1.0f);
	return std::max<float>(val * (1.0f - sustainNudge), 0.0f);
}

float nudgeEnvRelease(float val, bool dir) {
	constexpr float releaseNudge = 0.08f;
	if (dir)
		return std::min<float>(val * (1.0f + releaseNudge), RELEASE_MAX);
	return std::max<float>(val * (1.0f - releaseNudge), RELEASE_MIN);
}

// lfos -----

float nudgeLFOFreq(float val, bool dir) {
	constexpr float freqNudge = 0.08f;
	if (dir)
		return std::min<float>(val * (1.0f + freqNudge), LFO_FREQ_MAX);
	return std::max<float>(val * (1.0f - freqNudge), LFO_FREQ_MIN);
}

uint8_t nudgeLFOType(uint8_t val, bool dir) {
	if (dir)
		return std::min<uint8_t>(val + 1, NUM_LFO_TYPES - 1);
	return (val == 0) ? 0 : val - 1;
}

// oscs----------
int8_t nudgeCoarseTune(int8_t val, bool dir) {
	if (dir)
		return std::min<int8_t>(val + 1, COARSE_MAX);
	return std::max<int8_t>(val - 1, COARSE_MIN);
}

int8_t nudgeFineTune(int8_t val, bool dir) {
	if (dir)
		return std::min<int8_t>(val + 1, FINE_MAX);
	return std::max<int8_t>(val - 1, FINE_MIN);
}

uint16_t nudgePulseWidth(uint16_t val, bool dir) {
	if (dir)
		return std::min<uint16_t>(val + 1, PWM_MAX);
	return std::max<uint16_t>(val - 1, PWM_MIN);
}

uint8_t nudgeWaveLevel(uint8_t val, bool dir) {
	if (dir)
		return std::min<uint8_t>(val + 1, 255);
	return (val == 0) ? 0 : val - 1;
}

// filter---------------
uint16_t nudgeFilterCutoff(uint16_t val, bool dir) {
	constexpr float cutoffExp = 0.07f;
	float fVal;
	if (dir) {
		fVal = (float) val * (1.0f + cutoffExp);
		return std::min<uint16_t>((uint16_t) fVal, CUTOFF_MAX);
	}
	fVal = (float) val * (1.0f - cutoffExp);
	return std::max<uint16_t>((uint16_t) fVal, CUTOFF_MIN);
}

uint16_t nudgeFilterRes(uint16_t val, bool dir) {
	if (dir)
		return std::min<uint16_t>(val + 1, RES_MAX);
	return std::min<uint16_t>(val - 1, RES_MIN);
}

// folder
uint16_t nudgeFold(uint16_t val, bool dir) {
	constexpr float foldNudge = 0.085f;
	float fVal;
	if (dir) {
		fVal = (float) val * (1.0f + foldNudge);
		return std::min<uint16_t>((uint16_t) fVal, FOLD_MAX);
	}
	fVal = (float) val * (1.0f - foldNudge);
	return std::max<uint16_t>((uint16_t) fVal, FOLD_MIN);
}

void SynthProcessor::nudgeParameter(uint8_t id, bool dir) {
	switch (id) {
	// envelopes
	case ParamID::pEnv1Attack:
		patch.envs[0].attack = nudgeEnvAttack(patch.envs[0].attack, dir);
		break;
	case ParamID::pEnv1Decay:
		patch.envs[0].decay = nudgeEnvDecay(patch.envs[0].decay, dir);
		break;
	case ParamID::pEnv1Sustain:
		patch.envs[0].sustain = nudgeEnvSustain(patch.envs[0].sustain, dir);
		break;
	case ParamID::pEnv1Release:
		patch.envs[0].release = nudgeEnvRelease(patch.envs[0].release, dir);
		break;
	case ParamID::pEnv2Attack:
		patch.envs[1].attack = nudgeEnvAttack(patch.envs[1].attack, dir);
		break;
	case ParamID::pEnv2Decay:
		patch.envs[1].decay = nudgeEnvDecay(patch.envs[1].decay, dir);
		break;
	case ParamID::pEnv2Sustain:
		patch.envs[1].sustain = nudgeEnvSustain(patch.envs[1].sustain, dir);
		break;
	case ParamID::pEnv2Release:
		patch.envs[1].release = nudgeEnvRelease(patch.envs[1].release, dir);
		break;
		// LFOs
	case ParamID::pLFO1Freq:
		patch.lfos[0].freq = nudgeLFOFreq(patch.lfos[0].freq, dir);
		break;
	case ParamID::pLFO1Mode:
		patch.lfos[0].lfoType = nudgeLFOType(patch.lfos[0].lfoType, dir);
		break;
	case ParamID::pLFO2Freq:
		patch.lfos[1].freq = nudgeLFOFreq(patch.lfos[1].freq, dir);
		break;
	case ParamID::pLFO2Mode:
		patch.lfos[1].lfoType = nudgeLFOType(patch.lfos[1].lfoType, dir);
		break;
	case ParamID::pLFO3Freq:
		patch.lfos[2].freq = nudgeLFOFreq(patch.lfos[2].freq, dir);
		break;
	case ParamID::pLFO3Mode:
		patch.lfos[2].lfoType = nudgeLFOType(patch.lfos[2].lfoType, dir);
		break;
		// oscillator 1
	case ParamID::pOsc1Coarse:
		patch.oscs[0].coarseTune = nudgeCoarseTune(patch.oscs[0].coarseTune,
				dir);
		break;
	case ParamID::pOsc1Fine:
		patch.oscs[0].fineTune = nudgeFineTune(patch.oscs[0].fineTune, dir);
		break;
	case ParamID::pOsc1PulseWidth:
		patch.oscs[0].pulseWidth = nudgePulseWidth(patch.oscs[0].pulseWidth,
				dir);
		break;
	case ParamID::pOsc1SquareLevel:
		patch.oscs[0].pulseLevel = nudgeWaveLevel(patch.oscs[0].pulseLevel,
				dir);
		break;
	case ParamID::pOsc1SawLevel:
		patch.oscs[0].sawLevel = nudgeWaveLevel(patch.oscs[0].sawLevel, dir);
		break;
	case ParamID::pOsc1TriLevel:
		patch.oscs[0].triLevel = nudgeWaveLevel(patch.oscs[0].triLevel, dir);
		break;
	case ParamID::pOsc1OscLevel:
		patch.oscs[0].oscLevel = nudgeWaveLevel(patch.oscs[0].oscLevel, dir);
		break;
		// oscillator 2
	case ParamID::pOsc2Coarse:
		patch.oscs[1].coarseTune = nudgeCoarseTune(patch.oscs[1].coarseTune,
				dir);
		break;
	case ParamID::pOsc2Fine:
		patch.oscs[1].fineTune = nudgeFineTune(patch.oscs[1].fineTune, dir);
		break;
	case ParamID::pOsc2PulseWidth:
		patch.oscs[1].pulseWidth = nudgePulseWidth(patch.oscs[1].pulseWidth,
				dir);
		break;
	case ParamID::pOsc2SquareLevel:
		patch.oscs[1].pulseLevel = nudgeWaveLevel(patch.oscs[1].pulseLevel,
				dir);
		break;
	case ParamID::pOsc2SawLevel:
		patch.oscs[1].sawLevel = nudgeWaveLevel(patch.oscs[1].sawLevel, dir);
		break;
		break;
	case ParamID::pOsc2TriLevel:
		patch.oscs[1].triLevel = nudgeWaveLevel(patch.oscs[1].triLevel, dir);
		break;
	case ParamID::pOsc2OscLevel:
		patch.oscs[1].oscLevel = nudgeWaveLevel(patch.oscs[1].oscLevel, dir);
		break;
		// filter/folder
	case ParamID::pFilterCutoff:
		patch.cutoffBase = nudgeFilterCutoff(patch.cutoffBase, dir);
		break;
	case ParamID::pFilterRes:
		patch.resBase = nudgeFilterRes(patch.resBase, dir);
		break;
	case ParamID::pFilterMode:
		patch.highPassMode = (patch.highPassMode > 0) ? 0 : 1;
		break;
	case ParamID::pFoldLevel:
		patch.foldBase = nudgeFold(patch.foldBase, dir);
		break;
	case ParamID::pFoldFirst:
		patch.foldFirst = (patch.foldFirst > 0) ? 0 : 1;
		break;
	}
	graphicsProc->paramUpdated(id);
}


// mod depth nudging


void SynthProcessor::nudgeModDepth(mod_t* mod, bool dir){
	int8_t depth = get_mod_depth(*mod);
	if(dir){
		depth = (depth < 127) ? depth + 1 : 127;
	} else {
		depth = (depth > -127) ? depth - 1 : -127;
	}
	set_mod_depth(mod, depth);
}

// Encoders---------------
void SynthProcessor::handleViewEncoder(uint8_t enc, bool dir) {
	if (visibleView == ViewID::vEnv1) {
		switch (enc) {
		case EncID::A: // attack
			nudgeParameter(ParamID::pEnv1Attack, dir);
			break;
		case EncID::B: // decay
			nudgeParameter(ParamID::pEnv1Decay, dir);
			break;
		case EncID::C: // sustain
			nudgeParameter(ParamID::pEnv1Sustain, dir);
			break;
		case EncID::D: // release
			nudgeParameter(ParamID::pEnv1Release, dir);
			break;
		default:
			break;
		}
	} else if (visibleView == ViewID::vEnv2) {
		switch (enc) {
		case EncID::A:
			nudgeParameter(ParamID::pEnv2Attack, dir);
			break;
		case EncID::B:
			nudgeParameter(ParamID::pEnv2Decay, dir);
			break;
		case EncID::C:
			nudgeParameter(ParamID::pEnv2Sustain, dir);
			break;
		case EncID::D:
			nudgeParameter(ParamID::pEnv2Release, dir);
			break;
		default:
			break;
		}
	} else if (visibleView == ViewID::vMix1) {
		switch (enc) {
		case EncID::A: // square level
			nudgeParameter(ParamID::pOsc1SquareLevel, dir);
			break;
		case EncID::B: // saw level
			nudgeParameter(ParamID::pOsc1SawLevel, dir);
			break;
		case EncID::C: // tri level
			nudgeParameter(ParamID::pOsc1TriLevel, dir);
			break;
		case EncID::D: // osc level
			nudgeParameter(ParamID::pOsc1OscLevel, dir);
			break;
		default:
			break;
		}
	} else if (visibleView == ViewID::vMix2) {
		switch (enc) {
		case EncID::A:
			nudgeParameter(ParamID::pOsc2SquareLevel, dir);
			break;
		case EncID::B:
			nudgeParameter(ParamID::pOsc2SawLevel, dir);
			break;
		case EncID::C:
			nudgeParameter(ParamID::pOsc2TriLevel, dir);
			break;
		case EncID::D:
			nudgeParameter(ParamID::pOsc2OscLevel, dir);
			break;
		default:
			break;
		}
	} else if (visibleView == ViewID::vTune) {
		switch (enc) {
		case EncID::A: // osc 1 coarse
			nudgeParameter(ParamID::pOsc1Coarse, dir);
			break;
		case EncID::B: // osc 1 fine
			nudgeParameter(ParamID::pOsc1Fine, dir);
			break;
		case EncID::C:
			nudgeParameter(ParamID::pOsc2Coarse, dir);
			break;
		case EncID::D:
			nudgeParameter(ParamID::pOsc2Fine, dir);
			break;
		default:
			break;
		}
	}

}

void SynthProcessor::handleEncoderTurn(uint8_t num, uint8_t clockwise) {
	EncID id = (EncID) num;
	switch (id) {
	case A:
		handleViewEncoder(num, clockwise > 0);
		break;
	case B:
		handleViewEncoder(num, clockwise > 0);
		break;
	case C:
		handleViewEncoder(num, clockwise > 0);
		break;
	case D:
		handleViewEncoder(num, clockwise > 0);
		break;
	case MenuEnc:
		break;
	case Depth:
		if(selectedMod != nullptr){
			nudgeModDepth(selectedMod, clockwise > 0);
		}
		break;
	case Cutoff:
		nudgeParameter(ParamID::pFilterCutoff, clockwise > 0);
		break;
	case Res:
		nudgeParameter(ParamID::pFilterRes, clockwise > 0);
		break;
	case PWM:
		nudgeParameter(selectedPWM, clockwise > 0);
		break;
	case Fold:
		nudgeParameter(ParamID::pFoldLevel, clockwise > 0);
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
		visibleView = ViewID::vEnv1;
		graphicsProc->selectView(visibleView);
		break;
	case Env2:
		visibleView = ViewID::vEnv2;
		graphicsProc->selectView(visibleView);
		break;
	case LFO1:
		visibleView = ViewID::vLFO1;
		graphicsProc->selectView(visibleView);
		break;
	case LFO2:
		visibleView = ViewID::vLFO2;
		graphicsProc->selectView(visibleView);
		break;
	case LFO3:
		visibleView = ViewID::vLFO3;
		graphicsProc->selectView(visibleView);
		break;
	case Osc1:
		visibleView = alt() ? ViewID::vTune : ViewID::vMix1;
		graphicsProc->selectView(visibleView);
		break;
	case Osc2:
		visibleView = alt() ? ViewID::vTune : ViewID::vMix2;
		graphicsProc->selectView(visibleView);
		break;
	case PWMB: //TODO: some sort of modal component to indicate what this button is for
		selectedPWM =
				(selectedPWM == ParamID::pOsc1PulseWidth) ?
						ParamID::pOsc2PulseWidth : ParamID::pOsc1PulseWidth;
		break;
	case FilterMode:
		patch.highPassMode = (patch.highPassMode > 0) ? 0 : 1;
		graphicsProc->paramUpdated(ParamID::pFilterMode);
		break;
	case FoldFirst:
		patch.foldFirst = (patch.foldFirst > 0) ? 0 : 1;
		graphicsProc->paramUpdated(ParamID::pFoldFirst);
		break;
	case Menu:
		break;
	case MS1:
		processSrcClick(0);
		break;
	case MS2:
		processSrcClick(1);
		break;
	case MS3:
		processSrcClick(2);
		break;
	case MS4:
		processSrcClick(3);
		break;
	case MS5:
		processSrcClick(4);
		break;
	case MS6:
		processSrcClick(5);
		break;
	case ModBank:
		inBank2 = !inBank2;
		break;
	case MD1:
		processDestClick(0);
		break;
	case MD2:
		processDestClick(1);
		break;
	case MD3:
		processDestClick(2);
		break;
	case MD4:
		processDestClick(3);
		break;
	case MD5:
		processDestClick(4);
		break;
	case MD6:
		processDestClick(5);
		break;
	case MD7:
		processDestClick(6);
		break;
	case MD8:
		processDestClick(7);
		break;
	default:
		break;
	}
}

void SynthProcessor::handleOnPressStart(uint8_t button) {
	ButtonID id = (ButtonID) button;
	switch (id) {
	case Alt:
		inAltMode = true;
		//TODO: bring up the relevant menu for alt mode
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
		inAltMode = false;
		//TODO: we need tell the graphics processor to redraw the non-alt component here
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
		button_processor_t bp, graphics_processor_t gp, pixel_processor_t pp) {
	return new SynthProcessor(clk, ep, bp, gp, pp);
}

void update_dac_levels(synth_processor_t proc, dacLevels_t *levels) {
	SynthProcessor *ptr = static_cast<SynthProcessor*>(proc);
	ptr->updateDacLevels(levels);
}

void process_midi_msg(synth_processor_t proc, midi_t msg) {
	SynthProcessor *ptr = static_cast<SynthProcessor*>(proc);
	ptr->processMidiMessage(msg);
}

void check_gui_updates(synth_processor_t proc) {
	SynthProcessor *ptr = static_cast<SynthProcessor*>(proc);
	ptr->checkGUIUpdates();
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
