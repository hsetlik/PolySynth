/*
 * NeopixelChain.h
 *
 *  Created on: Sep 12, 2024
 *      Author: hayden
 */

#ifndef INC_NEOPIXELCHAIN_H_
#define INC_NEOPIXELCHAIN_H_

#include "main.h"
#include "stm32f4xx_hal.h"
#include <string.h>
// handle IDs for each channel
#define PX_TIMER_MAIN htim2
#define PX_TIMER_MATRIX htim5
#define PX_DMA_MAIN hdma_tim2_ch2_ch4
#define PX_DMA_MATRIX hdma_tim5_ch3_up

typedef enum {
	pxMatrix,
	pxMain
}pxChan;


#ifdef __cplusplus
#include "color.h"

#define DMA_BUF_LEDS 4 // send data to DMA in chunks of 4 pixels at at time
#define PWM_LOW 4
#define PWM_HIGH 8

class SK6812Chain {
private:
	TIM_HandleTypeDef* const timer;
	const uint8_t numLeds;
	const uint8_t finalLed;
	uint8_t currentLed = 0;
	bool dmaWorking = false;
	bool updateRequested = false;
	color32_t* colors;
	uint16_t dmaBuf[DMA_BUF_LEDS * sizeof(color32_t) * 8]; // this is 128 bytes in practice
	// this guy does most of the work
	void fillNextBuffer(uint16_t* buf);
public:
	SK6812Chain(TIM_HandleTypeDef* tim, uint8_t pixels);
	~SK6812Chain();
	void begin(DMA_HandleTypeDef* dma);
	// wire these to the appropriate callbacks in main.c
	void transmitHalfComplete();
	void transmitComplete();
	//----------
	void setPixel(uint8_t px, color32_t col);


};











#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_NEOPIXELCHAIN_H_ */
