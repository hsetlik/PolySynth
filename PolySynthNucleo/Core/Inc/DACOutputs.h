/*
 * DACOutputs.h
 *
 *  Created on: May 31, 2024
 *      Author: hayden
 */

#ifndef INC_DACOUTPUTS_H_
#define INC_DACOUTPUTS_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "DAC7578.h"

enum dac_chan_t {
	LVL1,
	LVL2,
	PWM1,
	PWM2,
	FOLD,
	VCA,
	CUTOFF
};

// set the GPIO such thatthe DAC outputs are multiplexed to voice v
void DAC_MUX_selectVoice(GPIO_TypeDef* port, uint8_t v);

void DAC_MUX_writeValue(GPIO_TypeDef* port, uint8_t voice, uint16_t value);



#ifdef __cplusplus
}
#endif



#endif /* INC_DACOUTPUTS_H_ */
