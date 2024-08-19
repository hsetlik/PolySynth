/*
 * ADSR.h
 *
 *  Created on: Aug 19, 2024
 *      Author: hayden
 */

#ifndef INC_ADSR_H_
#define INC_ADSR_H_

#include "main.h"
#include "Patch.h"

float tickLengthForTimer(TIM_Base_InitTypeDef* timebase);


#ifdef __cplusplus

class ADSRProcessor
{
private:
	adsr_t* const params;
	const float tickLengthMs;
public:
	ADSRProcessor(adsr_t* env);
};

#endif

#ifdef __cplusplus
#define EXTERNC extern "c"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_ADSR_H_ */
