/*
 * StatusLED.c
 *
 *  Created on: Nov 5, 2024
 *      Author: hayden
 */
#include "StatusLED.h"
#include <stdbool.h>

// little GPIO wrapper
void setLED(uint8_t led, uint8_t lvl){
	GPIO_PinState state = (GPIO_PinState)lvl;
	if(led < 1){
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, state);
	} else {
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, state);
	}
}

// static state what for keeping track of things
static uint8_t mode1 = 0;
static uint8_t mode2 = 0;
static bool pinLvl1 = false;
static bool pinLvl2 = false;
static tick_t lastSwitch1 = 0;
static tick_t lastSwitch2 = 0;
static bool mode1Changed = false;
static bool mode2Changed = false;
static bool isDormant = false;
//=============================
void StatusLED_setModes(uint8_t m1, uint8_t m2){
	if(m1 != mode1){
		mode1 = m1;
		mode1Changed = true;
	}
	if(m2 != mode2){
		mode2 = m2;
		mode2Changed = true;
	}
	isDormant = ~(mode1 | mode2);
}

void StatusLED_tick(tick_t now){

}
