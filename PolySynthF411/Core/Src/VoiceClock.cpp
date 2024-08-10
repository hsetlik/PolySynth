/*
 * VoiceClock.cpp
 *
 *  Created on: May 25, 2024
 *      Author: hayden
 */
#include "VoiceClock.h"


float Timing::getActualUpdateHz(uint32_t masterClk, uint32_t updateHz){
	uint32_t masterPeriods = masterClk / updateHz;
	return (float)masterClk / (float)masterPeriods;
}

float Timing::getTickHz(uint32_t clkFreq, uint8_t dataWidth){
	return (float)clkFreq / (float)dataWidth;
}

//========================================================================================
VoiceClock::VoiceClock() :
tickHz(Timing::getTickHz(MASTER_CLK, 16)), // NOTE: with 2.625 MHz bit clock tickHz is around 164 kHz
oscState(0)
{
	// initialize the oscs each with their own frequency
	constexpr float freqDelta = 4158.5f / (float)NUM_OSCS;
	constexpr float minFreq = 27.5f;
	for(uint8_t o = 0; o < NUM_OSCS; o++){
		float freq = minFreq + ((float)o * freqDelta);
		setFrequency(o, freq);
	}

}

void VoiceClock::setFrequency(uint8_t osc, float hz)
{
	fullTicks[osc] = (tick_t)(tickHz / hz);
	halfTicks[osc] = fullTicks[osc] / 2;
}

void VoiceClock::tick()
{
	for(uint8_t o = 0; o < NUM_OSCS; o++){
		tick_t& current = currentTicks[o];
		current = (current + 1) % fullTicks[o];
		bool prevState = oscState & (1 << o);
		if(prevState && current < halfTicks[o]){
			// clear the bit
			oscState = oscState & ~(1 << o);
		}
		else if(!prevState && current > halfTicks[o]){
			// set the bit
			oscState = oscState | (1 << o);
		}
	}
}

uint16_t VoiceClock::getNext(){
	tick();
	// shift left bc the last 4 bits are unused
	return oscState << 4;
}

//=========================================================================

voice_clock_t create_voice_clock()
{
	return new VoiceClock();
}

void destroy_voice_clock(voice_clock_t clk)
{
	VoiceClock* ptr = static_cast<VoiceClock*>(clk);
	delete ptr;
}

void set_osc_frequency(voice_clock_t clk, uint8_t osc, float hz)
{
	VoiceClock* ptr = static_cast<VoiceClock*>(clk);
	ptr->setFrequency(osc, hz);
}

uint16_t get_next_bits(voice_clock_t clk)
{
	VoiceClock* ptr = static_cast<VoiceClock*>(clk);
	return ptr->getNext();
}

void send_bits(voice_clock_t clk, uint16_t* dest, uint16_t startIdx, uint16_t endIdx)
{
	VoiceClock* ptr = static_cast<VoiceClock*>(clk);
	for(uint16_t idx = startIdx; idx < endIdx; idx++){
		dest[idx] = ptr->getNext();
	}
}




