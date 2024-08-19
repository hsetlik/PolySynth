/*
 * TickTimer.c
 *
 *  Created on: Aug 19, 2024
 *      Author: hayden
 */

#include "TickTimer.h"

uint32_t val = 0;

uint32_t TickTimer_get()
{
	return val;
}

void TickTimer_tick()
{
	val++;
}


