/*
 * DACOutputs.cpp
 *
 *  Created on: May 31, 2024
 *      Author: hayden
 */

#include "DACOutputs.h"

void DAC_MUX_selectVoice(GPIO_TypeDef* port, uint8_t v){
	static GPIO_PinState a0 = GPIO_PIN_RESET;
	static GPIO_PinState a1 = GPIO_PIN_RESET;
	static GPIO_PinState a2 = GPIO_PIN_RESET;
	//set the pins as needed
	if(v & (1 << 0))
		a0 = GPIO_PIN_SET;
	if(v & (1 << 1))
		a1 = GPIO_PIN_SET;
	if(v & (1 << 2))
		a2 = GPIO_PIN_SET;
	//write pins
	//HAL_GPIO_WritePin(port, DAC_A0_Pin, a0);
	//HAL_GPIO_WritePin(port, DAC_A1_Pin, a1);
	//HAL_GPIO_WritePin(port, DAC_A2_Pin, a2);
}


void DAC_MUX_writeValue(GPIO_TypeDef* port, uint8_t voice, uint16_t value){
	DAC_MUX_selectVoice(port, voice);

}




