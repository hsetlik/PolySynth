/*
 * MIDI.c
 *
 *  Created on: Jun 1, 2024
 *      Author: hayden
 */
#include "MIDI.h"

enum MIDIMsgType MIDI_getMsgType(uint8_t cmdByte){
	// right 4 bits store the channel so we clear them
	uint8_t mask = 0xF0;
	uint8_t left4 = cmdByte & mask;

	if(left4 != 0xF0){
		switch(left4){
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
		switch(cmdByte){
		case 0xF2:
			return SongPosition;
		case 0xF3:
			return SongSelect;
		case 0xF5:
			return BusSelect;
		case 0xF6:
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

uint8_t MIDI_getMsgChannel(uint8_t cmdByte){
	uint8_t mask = 255 >> 4;
	return cmdByte & mask;
}

midiMsg MIDI_decodeMsg(uint8_t* ptr){
	midiMsg output;
	output.msgType = (uint8_t)MIDI_getMsgType(ptr[0]);
	// if the message applies to a specific channel
	if(output.msgType < 0xF2){
		output.channel = MIDI_getMsgChannel(ptr[0]);
	}
	output.data[0] = ptr[1];
	output.data[1] = ptr[2];

	return output;
}


