/*
 * MIDI.h
 *
 *  Created on: Jun 1, 2024
 *      Author: hayden
 */

#ifndef INC_MIDI_H_
#define INC_MIDI_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "main.h"
// the possible message types
enum MIDIMsgType{
	NoteOff,
	NoteOn,
	KeyPressure,
	ControlChange,
	ProgramChange,
	ChannelPressure,
	PitchBend,
	SongPosition,
	SongSelect,
	BusSelect,
	TuneRequest,
	TimingTick,
	StartSong,
	ContinueSong,
	StopSong,
	ActiveSensing,
	SystemReset
};

typedef struct {
	uint8_t msgType;
	uint8_t channel;
	uint8_t data[2];
}midiMsg;

enum MIDIMsgType MIDI_getMsgType(uint8_t cmdByte);
uint8_t MIDI_getMsgChannel(uint8_t cmdByte);
midiMsg MIDI_decodeMsg(uint8_t* ptr);

// TUNING STUFF =======================================================================
#define MAX_MIDI_NUM 120
#define SEMITONE_RATIO 1.05946309436f

// the heavy lifting of tuning
float hzForTuning(uint8_t note, int8_t semitones, int8_t cents);
uint16_t dacValueForHz(float hz);


#ifdef __cplusplus
}
#endif

#endif /* INC_MIDI_H_ */
