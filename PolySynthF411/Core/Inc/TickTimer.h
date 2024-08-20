/*
 * TickTimer.h
 *
 *  Created on: Aug 19, 2024
 *      Author: hayden
 */

#ifndef INC_TICKTIMER_H_
#define INC_TICKTIMER_H_
#include <stdint.h>
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t tick_t;

tick_t TickTimer_get();
void TickTimer_tick();
void TickTimer_setTickTime(TIM_Base_InitTypeDef* timebase);
float TickTimer_getTickTimeMs();
float TickTimer_tickDistanceMs(tick_t first, tick_t second);


#ifdef __cplusplus
}
#endif

#endif /* INC_TICKTIMER_H_ */
