/*
 * VoiceClock.h
 *
 *  Created on: May 25, 2024
 *      Author: hayden
 */

#ifndef INC_VOICECLOCK_H_
#define INC_VOICECLOCK_H_


#define NUM_OSCS 12
#define MASTER_CLK 2625000

#define BUFFER_SIZE 256

#include "stm32f4xx_hal.h"
#include "main.h"

// protect the C++ stuff
#ifdef __cplusplus

typedef uint16_t tick_t;
// helpers
namespace Timing{
float getActualUpdateHz(uint32_t masterClk, uint32_t updateHz);
float getTickHz(uint32_t clkFreq, uint8_t dataWidth);
};

class VoiceClock
{
private:
	// helper for timing math
	const float tickHz;

	// oscillator state stuff
	tick_t fullTicks[NUM_OSCS];
	tick_t halfTicks[NUM_OSCS];
	tick_t currentTicks[NUM_OSCS];
	uint16_t oscState;

	// the heavy lifting
	void tick();

public:
	VoiceClock();
	void setFrequency(uint8_t osc, float hz);
	uint16_t getNext();
};
#endif // __cplusplus


// wrapper we need to call this stuff from main.c
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef void* voice_clock_t;

EXTERNC voice_clock_t create_voice_clock();
EXTERNC void destroy_voice_clock(voice_clock_t clk);
EXTERNC void set_osc_frequency(voice_clock_t clk, uint8_t osc, float hz);
EXTERNC uint16_t get_next_bits(voice_clock_t clk);
EXTERNC void send_bits(voice_clock_t clk, uint16_t* dest, uint16_t startIdx, uint16_t endIdx);

#undef EXTERNC

#endif /* INC_VOICECLOCK_H_ */
