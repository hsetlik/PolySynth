/*
 * VoiceClock.cpp
 *
 *  Created on: May 25, 2024
 *      Author: hayden
 */
#include "VoiceClock.h"
#include <cmath>

float Timing::getActualUpdateHz(uint32_t masterClk, uint32_t updateHz) {
	uint32_t masterPeriods = masterClk / updateHz;
	return (float) masterClk / (float) masterPeriods;
}

float Timing::getTickHz(uint32_t clkFreq, uint8_t dataWidth) {
	return (float) clkFreq / (float) dataWidth;
}

//========================================================================================
VoiceClock::VoiceClock() :
		tickHz(0), // NOTE: with 2.625 MHz bit clock tickHz is around 164 kHz
		oscState(0) {


}

void VoiceClock::setFrequency(uint8_t osc, float hz) {
#ifndef FLOAT_VOICE_MODE
	fullTicks[osc] = (voice_tick_t) (tickHz / hz);
	halfTicks[osc] = fullTicks[osc] / 2;
#else
	phaseDelta[osc] = hz / tickHz;
#endif
}

void VoiceClock::tick() {
#ifndef FLOAT_VOICE_MODE
	for (uint8_t o = 0; o < NUM_OSCS; o++) {
		voice_tick_t &current = currentTicks[o];
		current = (current + 1) % fullTicks[o];
		bool prevState = oscState & (1 << o);
		if (prevState && current < halfTicks[o]) {
			// clear the bit
			oscState = oscState & ~(1 << o);
		} else if (!prevState && current > halfTicks[o]) {
			// set the bit
			oscState = oscState | (1 << o);
		}
	}
#else
	for(uint8_t osc = 0; osc < NUM_OSCS; osc++){
		// waves are high when phase > 0.5
		phase[osc] = std::fmod(phase[osc] + phaseDelta[osc], 1.0f);
		bool prevState = oscState & (1 << osc);
		if(prevState && phase[osc] < 0.5f){ // clear the bit
			oscState = oscState & ~(1 << osc);
		}
		else if (!prevState && phase[osc] >= 0.5f){ // set the bit
			oscState = oscState | (1 << osc);
		}
	}

#endif
}

uint16_t VoiceClock::getNext() {
	tick();
	return oscState << (16 - NUM_OSCS);
}

void VoiceClock::fillHalfBuffer(uint16_t *buf) {
	for (uint8_t s = 0; s < VOICE_CLOCK_BUF_SIZE / 2; s++) {
		buf[s] = getNext();
	}
}

void VoiceClock::begin(SPI_HandleTypeDef *spi) {
	// 1. calculate the timing frequency
	tickHz = (float)APB2_CLK / (float)spi->Init.BaudRatePrescaler / 16.0f;
	// initialize the oscs each with their own frequency
	constexpr float freqDelta = 4158.5f / (float) NUM_OSCS;
	constexpr float minFreq = 27.5f;
	for (uint8_t o = 0; o < NUM_OSCS; o++) {
		float freq = minFreq + ((float) o * freqDelta);
		setFrequency(o, freq);
	}
	// 2: fill the first half of what will be the DMA buffer
	fillHalfBuffer(dmaBuf);
	fillHalfBuffer(&dmaBuf[VOICE_CLOCK_BUF_SIZE / 2]);
	// 3. start the DMA transmission
	if (!HAL_SPI_Transmit_DMA(spi, (uint8_t*) dmaBuf,
	VOICE_CLOCK_BUF_SIZE * 2)) {
		Error_Handler();
	}
}

void VoiceClock::txHalfFinished() {
	// we've just finished transmitting the first half of the buffer so put
	// the new values there while the second half sends
	fillHalfBuffer(dmaBuf);
}

void VoiceClock::txFinished() {
	// and vice versa when we finish the second half
	fillHalfBuffer(&dmaBuf[VOICE_CLOCK_BUF_SIZE / 2]);
}

//=========================================================================

voice_clock_t create_voice_clock() {
	return new VoiceClock();
}

void destroy_voice_clock(voice_clock_t clk) {
	VoiceClock *ptr = static_cast<VoiceClock*>(clk);
	delete ptr;
}

void set_osc_frequency(voice_clock_t clk, uint8_t osc, float hz) {
	VoiceClock *ptr = static_cast<VoiceClock*>(clk);
	ptr->setFrequency(osc, hz);
}

void begin(voice_clock_t clk, SPI_HandleTypeDef *spi) {
	VoiceClock *ptr = static_cast<VoiceClock*>(clk);
	ptr->begin(spi);
}

void tx_half_complete(voice_clock_t clk){
	VoiceClock *ptr = static_cast<VoiceClock*>(clk);
	ptr->txHalfFinished();
}

void tx_complete(voice_clock_t clk){
	VoiceClock *ptr = static_cast<VoiceClock*>(clk);
	ptr->txFinished();
}
