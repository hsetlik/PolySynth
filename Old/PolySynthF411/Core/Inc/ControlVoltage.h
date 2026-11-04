/*
 * ControlVoltage.h
 *
 *  Created on: Jul 25, 2024
 *      Author: hayden
 */

#ifndef INC_CONTROLVOLTAGE_H_
#define INC_CONTROLVOLTAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "DAC7578.h"
#include "stm32f4xx_hal.h"

// our 7 control voltages in order of DAC channel
enum DACChannel{
	AC1_CH,
	AC2_CH,
	PWM2_CH,
	VCA_CH,
	PWM1_CH,
	FOLD_CH,
	CUTOFF_CH
};

// convert output voltages and parameter values to 12-bit DAC levels

// for floats in the range 0-1
uint16_t dacValueForNorm(float value);
// the needed value to compensate for a given frequency
uint16_t dacValueForDCO(float hz);

// 7 12-bit values for our 7 control voltages
typedef struct{
	uint16_t prevData[7];
	uint16_t currentData[7];
} dacLevels_t;

void updateVoiceCV(I2C_HandleTypeDef* i2c, uint16_t* newLevels, uint16_t* prevLevels, uint8_t voice);
void updateVoiceCV_DMA(I2C_HandleTypeDef* i2c, uint16_t* newLevels, uint16_t* prevLevels, uint8_t voice);
void startVoiceUpdate_DMA(I2C_HandleTypeDef* i2c, uint16_t* newLevels, uint16_t* prevLevels);
void finishVoiceUpdate_DMA(uint8_t voice);



#ifdef __cplusplus
}
#endif

#endif /* INC_CONTROLVOLTAGE_H_ */
