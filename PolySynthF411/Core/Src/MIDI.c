/*
 * MIDI.c
 *
 *  Created on: Jun 1, 2024
 *      Author: hayden
 */
#include "MIDI.h"
#include <math.h>

uint8_t isStatusByte(uint8_t byte){
	return byte & 0x80;
}

MIDIMsgType getMsgType(uint8_t cmdByte) {
	// right 4 bits store the channel so we clear them
	uint8_t left4 = cmdByte & 0xF0;

	if (left4 != 0xF0) {
		switch (left4) {
		case 0x80:
			return NoteOff;
		case 0x90:
			return NoteOn;
		case 0xA0:
			return KeyPressure;
		case 0xB0:
			return ControlChange;
		case 0xC0:
			return ProgramChange;
		case 0xD0:
			return ChannelPressure;
		case 0xE0:
			return PitchBend;
		default:
			return ControlChange;
		}
	} else {
		switch (cmdByte) {
		case 0xF2:
			return SongPosition;
		case 0xF3:
			return SongSelect;
		case 0xF5:
			return TuneRequest;
		case 0xF8:
			return TimingTick;
		case 0xFA:
			return StartSong;
		case 0xFB:
			return ContinueSong;
		case 0xFC:
			return StopSong;
		case 0xFE:
			return ActiveSensing;
		case 0xFF:
			return SystemReset;
		default:
			return SystemReset;
		}

	}
}

// the number of control bytes in each type of message
uint8_t dataBytesForMessageType(uint8_t msg){
	MIDIMsgType msgType = getMsgType(msg);
	switch(msgType){
	case NoteOff:
		return 2;
	case NoteOn:
		return 2;
	case KeyPressure:
		return 2;
	case ControlChange:
		return 2;
	case ProgramChange:
		return 1;
	case ChannelPressure:
		return 1;
	case PitchBend:
		return 2;
	case SongPosition:
		return 2;
	case SongSelect:
		return 1;
	case TuneRequest:
		return 0;
	case TimingTick:
		return 0;
	case StartSong:
		return 0;
	case ContinueSong:
		return 0;
	case StopSong:
		return 0;
	case ActiveSensing:
		return 0;
	case SystemReset:
		return 0;
	default:
		return 1;
	}

}

uint8_t getStatusChannel(uint8_t cmdByte) {

	return cmdByte & 0x0F;
}

// MIDI state machine======================================

// state storage
uint8_t statusByte = 0x00;
uint8_t dataBytes[3];
uint8_t dataBytesReceived = 0;
uint8_t dataBytesExpected = 0;

// helper for MIDI_receiveByte
uint8_t isMessageComplete(){
	if(!statusByte)
		return 0;
	return dataBytesReceived == dataBytesExpected;
}

uint8_t MIDI_receiveByte(uint8_t data){
	if(isStatusByte(data)){ // new status byte
		statusByte = data;
		dataBytesReceived = 0;
		dataBytesExpected = dataBytesForMessageType(statusByte);
	} else { // a data byte
		dataBytes[dataBytesReceived] = data;
		++dataBytesReceived;
	}
	if(isMessageComplete()){
		dataBytesReceived = 0;
		return 1;
	}
	return 0;
}

midi_t MIDI_getLatestMessage(){
	midi_t msg;
	msg.msgType = getMsgType(statusByte);
	msg.channel = getStatusChannel(statusByte);
	msg.data[0] = dataBytes[0];
	msg.data[1] = dataBytes[1];
	return msg;
}

//=========================================================

float hzForTuning(uint8_t note, int8_t semitones, int8_t cents) {
	float fNote = (float) note - 69.0f;
	fNote += (float) semitones + ((float) cents / 100.0f);
	return 440.0f * powf(SEMITONE_RATIO, fNote);
}


uint16_t dacValueForHz(float hz){
	// the hz value for which our DAC should be set to 4096
	static const float maxHz = 8123.0f;
	float fDacVal = (hz / maxHz) * 4096.0f;
	return (uint16_t)fDacVal;

}

