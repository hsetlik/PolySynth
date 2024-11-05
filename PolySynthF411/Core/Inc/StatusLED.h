/*
 * StatusLED.h
 *
 *  Created on: Nov 5, 2024
 *      Author: hayden
 */

#ifndef INC_STATUSLED_H_
#define INC_STATUSLED_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
// proper declaration of this extern from main.c
void StatusLED_setModes(uint8_t mode1, uint8_t mode2);

// call this in the main while loop
void StatusLED_tick(tick_t now);



#ifdef __cplusplus
}
#endif

#endif /* INC_STATUSLED_H_ */
