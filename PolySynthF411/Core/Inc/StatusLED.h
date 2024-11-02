/*
 * StatusLED.h
 *
 *  Created on: Nov 2, 2024
 *      Author: hayden
 */

#ifndef INC_STATUSLED_H_
#define INC_STATUSLED_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"



typedef enum{
	off,
	solid,
	slowBlink,
	fastBlink
}StatusLedMode;

void StatusLED_off(uint8_t led);
void StatusLED_on(uint8_t led);
void StatusLED_slowBlink(uint8_t led);
void StatusLED_fastBlink(uint8_t led);

void StatusLED_tick(tick_t now);

#ifdef __cplusplus
}
#endif

#endif /* INC_STATUSLED_H_ */
