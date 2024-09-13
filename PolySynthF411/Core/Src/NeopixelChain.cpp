/*
 * NeopixelChain.cpp
 *
 *  Created on: Sep 12, 2024
 *      Author: hayden
 */
#include "NeopixelChain.h"

void colorToPWM(uint16_t* buf, color32_t col){
	for(uint8_t i = 0; i < sizeof(color32_t); i++){
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
	if(currentLed >= numLeds){

	} else {

	}
}

void SK6812Chain::transmitComplete(){

}

void SK6812Chain::transmitHalfComplete(){

}

void SK6812Chain::setPixel(uint8_t px, color32_t col){
	colors[px] = col;
}

void SK6812Chain::update(){

}
