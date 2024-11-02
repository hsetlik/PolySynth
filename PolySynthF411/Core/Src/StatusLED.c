/*
 * StatusLED.c
 *
 *  Created on: Nov 2, 2024
 *      Author: hayden
 */
#include "StatusLED.h"




static uint8_t mode1 = 0;
static uint8_t mode2 = 0;
static uint8_t level1 = 0;
static uint8_t level2 = 0;
static uint8_t modeChanged = 0;
static uint8_t isBlinking1 = 0;
static uint8_t isBlinking2 = 0;
static uint8_t lastChangedLED = 0;


// helper for the HAL calls
void setLED(uint8_t led, uint8_t level){
	GPIO_TypeDef* gpio;
	uint16_t pin;
	if(led > 0){
		gpio = LED2_GPIO_Port;
		pin = LED2_Pin;
		level2 = level;
	} else {
		gpio = LED1_GPIO_Port;
		pin = LED1_Pin;
		level1 = level;
	}
	HAL_GPIO_WritePin(gpio, pin, level);
}


// Forward declarations from main.h=============================
void setMode(uint8_t led, uint8_t state) {
	uint8_t *mode = (led > 0) ? &mode2 : &mode1;
	uint8_t *nmode = (led > 0) ? &mode1 : &mode2;
	if (*mode != state) {
		modeChanged = 1;
		lastChangedLED = led;
	}
	*mode = state;

	if (state > 1 || *nmode > 1) {
		eitherBlinking = 1;
	} else {
		eitherBlinking = 0;
	}

}

void StatusLED_off(uint8_t led) {
	setMode(led, 0);
}
void StatusLED_on(uint8_t led) {
	setMode(led, 1);
}
void StatusLED_slowBlink(uint8_t led){
	setMode(led, 2);
}
void StatusLED_fastBlink(uint8_t led){
	setMode(led, 3);
}
// Thic gets called from the main while loop=============

static tick_t lastSwitch1 = 0;
static tick_t lastSwitch2 = 0;
void StatusLED_tick(tick_t now) {
	if(modeChanged){
		uint8_t* mode = (lastChangedLED > 0) ? &mode2 : &mode1;
		modeChanged = 0;
	} else if(eitherBlinking){
		//TODO: check if it's time to toggle either blinking LED

	}

}

