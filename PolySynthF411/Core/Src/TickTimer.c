/*
 * TickTimer.c
 *
 *  Created on: Aug 19, 2024
 *      Author: hayden
 */

#include "TickTimer.h"

tick_t val = 0;

tick_t TickTimer_get() {
	return val;
}

void TickTimer_tick() {
	val++;
}

float tickTimeMs = 0.0f;

void TickTimer_setTickTime(TIM_Base_InitTypeDef *timebase) {
	/**
	 * this is based on pg11 of ST's AN1430:
	 * https://www.st.com/resource/en/application_note/an4013-introduction-to-timers-for-stm32-mcus-stmicroelectronics.pdf
	 *
	 */
	static const float timClk = 48000000.0f; // the base frequency for APB1 timers
	float psc = (float)timebase->Prescaler + 1;
	float arp = (float)timebase->AutoReloadPreload + 1;
	float rcr = (float)timebase->RepetitionCounter + 1;
	float hz = timClk / (psc * arp * rcr);
	// with current settings tickTimeMs should be about 0.2
	tickTimeMs = 1000.0f / hz;

}
float TickTimer_getTickTimeMs(){
	return tickTimeMs;
}


float TickTimer_tickDistanceMs(tick_t first, tick_t second){
	static const tick_t TICK_MAX = ~(tick_t)0;
	if(second > first){
		return (float)(second - first) * tickTimeMs;
	}
	/** this handles timer overflow
	 * our update frequency is 5000hz, therefore:
	 *  - a 16 bit counter overflows every 13 seconds
	 *  - a 32 bit counter overflows every 238 hours (a little less than 10 days)
	 *
	 *  math crazy, guess I'm sticking to 32 bit for the sake of performance?
	 *
	 */
	return (float)(second + (TICK_MAX - first)) * tickTimeMs;

}

