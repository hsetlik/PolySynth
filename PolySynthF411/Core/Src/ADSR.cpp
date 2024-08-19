/*
 * ADSR.cpp
 *
 *  Created on: Aug 19, 2024
 *      Author: hayden
 */




#include "ADSR.h"


float tickLengthForTimer(TIM_Base_InitTypeDef* timebase)
{
	/**
	 * this is based on pg11 of ST's AN1430:
	 * https://www.st.com/resource/en/application_note/an4013-introduction-to-timers-for-stm32-mcus-stmicroelectronics.pdf
	 *
	 */
	static const float timClk = 32000000.0f; // the base frequency for APB1 timers
	float psc = (float)timebase->Prescaler + 1;
	float arp = (float)timebase->AutoReloadPreload + 1;
	float rcr = (float)timebase->RepetitionCounter + 1;
	float hz = timClk / (psc * arp * rcr);
	return 1000.0f / hz;
}
