/*
 * NeopixelChain.h
 *
 *  Created on: Sep 12, 2024
 *      Author: hayden
 *
 *
 */


//  SK6812 driver and processor based on the datasheet from here:
//  https://www.digikey.com/htmldatasheets/production/1857559/0/0/1/SK6812RGBW-Specification.pdf

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

typedef enum {
	pxEnv1,
	pxEnv2,
	pxLFO1,
	pxLFO2,
	pxLFO3,
	pxOsc1,
	pxOsc2,
	pxCutoff,
	pxRes,
	pxPulseWidth,
	pxFold,
	pxFilterMode,
	pxPWM1,
	pxPWM2,
	pxFoldFirst
} pixelID;

#ifdef __cplusplus
#include "color.h"
#include <memory>

#define DMA_BUF_LEDS 4 // send data to DMA in chunks of 4 pixels at at time
#define PWM_LOW 24
#define PWM_HIGH 48

#define PIXEL_UPDATE_MS 25.0f

class SK6812Chain {
private:
	TIM_HandleTypeDef* const timer;
	const uint8_t numLeds;
	const uint8_t finalLed;
	uint8_t currentLed = 0;
	bool dmaWorking = false;
	bool colorsChanged = false;
	bool resetCodeSent = false;
	tick_t lastComplete = 0;
	uint16_t dmaBuf[DMA_BUF_LEDS * sizeof(color32_t) * 8]; // this is 128 bytes in practice
	std::unique_ptr<color32_t[]> colors;
	// this guy does most of the work
	void fillNextBuffer(uint16_t* buf);
public:
	SK6812Chain(TIM_HandleTypeDef* tim, uint8_t pixels);
	void begin();
	// wire these to the appropriate callbacks in main.c
	void transmitHalfComplete();
	void transmitComplete();
	void tick();
	//----------
	void setPixel(uint8_t px, color32_t col);
	color32_t getPixel(uint8_t px){
		return colors[px];
	}
};

//============================================================


class PixelProcessor {
private:
	SK6812Chain mainPx;
	SK6812Chain matrixPx;
public:
	PixelProcessor();
	// callbacks for main.c
	void tick();
	void begin();
	void txComplete(TIM_HandleTypeDef* tim);
	void txHalfComplete(TIM_HandleTypeDef* tim);

	// NOTE: the logic for the actual pixel colors goes here VVV
	void updateForCutoff(uint16_t val);
	void updateForRes(uint16_t val);
	void updateForFold(uint16_t val);

	// color getters/setters
	void setMainPixel(uint8_t id, color32_t color);
	void setMatrixPixel(uint8_t src, uint8_t dest, color32_t color);
	void setSourceBankPixel(uint8_t right, color32_t color);
	color32_t getMainPixel(uint8_t id);
	color32_t getMatrixPixel(uint8_t src, uint8_t dest);
	color32_t getSourceBankPixel(uint8_t right);
};







#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

typedef void* pixel_processor_t;

EXTERNC pixel_processor_t create_pixel_processor();
EXTERNC void tick_pixel_processor(pixel_processor_t proc);
EXTERNC void pixel_tx_half_complete(pixel_processor_t proc, TIM_HandleTypeDef* tim);
EXTERNC void pixel_tx_complete(pixel_processor_t proc, TIM_HandleTypeDef* tim);
EXTERNC void begin_pixels(pixel_processor_t proc);

#undef EXTERNC

#endif /* INC_NEOPIXELCHAIN_H_ */
