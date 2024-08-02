/*
 * ControlVoltage.c
 *
 *  Created on: Jul 25, 2024
 *      Author: hayden
 */

#include "ControlVoltage.h"
#include "main.h"
// helper to connect the TMUX4051 multiplexer to a given voice

void switchMuxToVoice(uint8_t voice){
	// the switch levels are the 3 LSBs of an 8 bit int
	uint8_t a = voice & 1;
	uint8_t b = voice & (1 << 1);
	uint8_t c = voice & (1 << 2);
	// to GPIO
	HAL_GPIO_WritePin(GPIOB, DAC_A0_Pin, (GPIO_PinState)a);
	HAL_GPIO_WritePin(GPIOB, DAC_A1_Pin, (GPIO_PinState)b);
	HAL_GPIO_WritePin(GPIOB, DAC_A2_Pin, (GPIO_PinState)c);
}


void updateVoiceCV(I2C_HandleTypeDef* i2c, uint16_t* levels, uint16_t* prevLevels, uint8_t voice){

	HAL_GPIO_WritePin(GPIOB, DAC_INH_Pin, GPIO_PIN_SET); // disconnect the DAC from the mux outputs
	// update the DAC
	for(uint8_t i = 0; i < 7; i++){
		// only update if the levels have changed
		if(levels[i] != prevLevels[i]){
			prevLevels[i] = levels[i];
			DAC7578_setLevel(i2c, i, levels[i]);
		}
	}
	// switch the muxes to the correct voice
	switchMuxToVoice(voice);
	// and finally pull the inhibit pin low
	HAL_GPIO_WritePin(GPIOB, DAC_INH_Pin, GPIO_PIN_RESET);

}
//

void updateVoiceCV_DMA(I2C_HandleTypeDef* i2c, uint16_t* levels, uint16_t* prevLevels, uint8_t voice){
	HAL_GPIO_WritePin(GPIOB, DAC_INH_Pin, GPIO_PIN_SET); // disconnect the DAC from the mux outputs

}


void startVoiceUpdate_DMA(I2C_HandleTypeDef* i2c, uint16_t* newLevels, uint16_t* prevLevels){
	// 1. inhibit the muxes
	HAL_GPIO_WritePin(GPIOB, DAC_INH_Pin, GPIO_PIN_SET); // disconnect the DAC from the mux outputs

	for(uint8_t i = 0; i < 7; i++){
		// only update if the levels have changed
		if(levels[i] != prevLevels[i]){
			prevLevels[i] = levels[i];
			DAC7578_setLevel(i2c, i, levels[i]);
		}
	}

}


void finishVoiceUpdate_DMA(uint8_t voice){
	switchMuxToVoice(voice);
	HAL_GPIO_WritePin(GPIOB, DAC_INH_Pin, GPIO_PIN_RESET);
}

uint16_t dacValueForNorm(float value){
	return (uint16_t) value * 4095.0f;
}

#define DCO_HZ_MAX 8173.0f

uint16_t dacValueForDCO(float hz){
	float fVal = hz / 8173.0f;
	return (uint16_t)fVal * 4095.0f;
}
