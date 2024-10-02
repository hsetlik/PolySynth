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
typedef enum {
	NoteOff,
	NoteOn,
	KeyPressure,
	ControlChange,
	ProgramChange,
	ChannelPressure,
	PitchBend,
	SongPosition,
	SongSelect,
	TuneRequest,
	TimingTick,
	StartSong,
	ContinueSong,
	StopSong,
	ActiveSensing,
	SystemReset
}MIDIMsgType;

typedef struct {
	uint8_t msgType;
	uint8_t channel;
	uint8_t data[2];
}midi_t;

// add a byte to the current midi message. returns true if there's
// a complete message to process at the end of this byte
uint8_t MIDI_receiveByte(uint8_t d);
// parse the most recent message
midi_t MIDI_getLatestMessage();


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
