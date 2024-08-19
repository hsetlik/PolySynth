/*
 * TickTimer.h
 *
 *  Created on: Aug 19, 2024
 *      Author: hayden
 */

#ifndef INC_TICKTIMER_H_
#define INC_TICKTIMER_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t TickTimer_get();
void TickTimer_tick();


#ifdef __cplusplus
}
#endif

#endif /* INC_TICKTIMER_H_ */
