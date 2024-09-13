/*
 * NeopixelChain.cpp
 *
 *  Created on: Sep 12, 2024
 *      Author: hayden
 */
#include "NeopixelChain.h"

void colorToPWM(uint16_t* buf, color32_t col){
	for(uint8_t i = 0; i < sizeof(color32_t) * 8; i++){
		if(col & (0x0001 << i)){
			buf[i] = PWM_HIGH;
		} else {
			buf[i] = PWM_LOW;
		}
	}
}




//=================================================
SK6812Chain::SK6812Chain(TIM_HandleTypeDef *tim, uint8_t pixels) :
		timer(tim), numLeds(pixels), finalLed(pixels + 10) {
	colors = new color32_t[numLeds];
}

SK6812Chain::~SK6812Chain(){
	delete colors;
}

void SK6812Chain::begin(DMA_HandleTypeDef* dma){

}

void SK6812Chain::fillNextBuffer(uint16_t* buf){
	constexpr uint8_t pixelsToSend = DMA_BUF_LEDS / 2; // such that we send half a buffer at a time
	if(currentLed >= numLeds){ // we've transmitted the last pixel
		memset(buf, 0x00, sizeof(color32_t) * sizeof(uint16_t));
	} else {
		uint8_t colorsSent = 0;
		while(colorsSent < pixelsToSend && currentLED < numLeds){
			colorToPWM(buf + (colorsSent * 2), colors[currentLed]);
			++currentLed;
			++colorsSent;
		}
	}
	// and increment the current LED
	++currentLed;
	currentLed %= finalLed;
}

void SK6812Chain::transmitComplete(){
	fillNextBuffer(dmaBuf);
}

void SK6812Chain::transmitHalfComplete(){
	fillNextBuffer(&dmaBuf[DMA_BUF_LEDS * sizeof(color32_t) * 4]);
}

void SK6812Chain::setPixel(uint8_t px, color32_t col){
	colors[px] = col;
}
