/*
 * StatusLED.c
 *
 *  Created on: Nov 5, 2024
 *      Author: hayden
 */
#include "StatusLED.h"
#include <stdbool.h>

// little GPIO wrapper
void setLED(uint8_t led, uint8_t lvl) {
	GPIO_PinState state = (GPIO_PinState) lvl;
	if (led < 1) {
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, state);
	} else {
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, state);
	}
}

// state for the two LEDs
led_mode_t mode1 = ledOff;
led_mode_t mode2 = ledOff;
uint8_t eitherBlinking = 0;
tick_t lastChange1 = 0;
tick_t lastChange2 = 0;
uint8_t currentLevel1 = 0;
uint8_t currentLevel2 = 0;

//=============================
void StatusLED_setModes(uint8_t mode1ID, uint8_t mode2ID) {
	led_mode_t m1 = (led_mode_t) mode1ID;
	led_mode_t m2 = (led_mode_t) mode2ID;
	// now check if each LED has changed
	if (mode1 != m1) {
		mode1 = m1;
		if (mode1 == ledOff) {
			setLED(0, 0);
			currentLevel1 = 0;
		} else {
			setLED(0, 1);
			currentLevel1 = 1;
		}
	}
	if (mode2 != m2) {
		mode2 = m2;
		if (mode2 == ledOff) {
			setLED(1, 0);
			currentLevel2 = 0;
		} else {
			setLED(1, 1);
			currentLevel2 = 1;
		}
	}
	// now check if we need to listen for blink updates
	if (mode1ID > 1 || mode2ID > 1) {
		eitherBlinking = 1;
	} else {
		eitherBlinking = 0;
	}
}

void StatusLED_tick(tick_t now) {
	if (eitherBlinking) {
		// constants for blink speed
		const float slowPeriodMs = 800.0f;
		const float fastPeriodMs = 250.0f;
		// check if LED 1 is blinking
		if (mode1 >= ledSlow) {
			const float period =
					(mode1 == ledSlow) ? slowPeriodMs : fastPeriodMs;
			float elapsed = TickTimer_tickDistanceMs(lastChange1, now);
			if(elapsed >= period){
				lastChange1 = now;
				currentLevel1 = (currentLevel1 < 1) ? 1 : 0;
				setLED(0, currentLevel1);
			}
		}
		// do the same for LED 2
		if (mode1 >= ledSlow) {
			const float period =
					(mode2 == ledSlow) ? slowPeriodMs : fastPeriodMs;
			float elapsed = TickTimer_tickDistanceMs(lastChange2, now);
			if(elapsed >= period){
				lastChange2 = now;
				currentLevel2 = (currentLevel2 < 1) ? 1 : 0;
				setLED(1, currentLevel2);

			}
		}
	}

}
