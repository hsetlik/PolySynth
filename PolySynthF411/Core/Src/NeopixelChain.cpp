/*
 * NeopixelChain.cpp
 *
 *  Created on: Sep 12, 2024
 *      Author: hayden
 */
#include "NeopixelChain.h"
#include <cmath>

void colorToPWM(uint16_t *buf, color32_t col) {
	for (uint8_t i = 0; i < sizeof(color32_t) * 8; i++) {
		if (col & (0x0001 << i)) {
			buf[i] = PWM_HIGH;
		} else {
			buf[i] = PWM_LOW;
		}
	}
}

//=================================================
SK6812Chain::SK6812Chain(TIM_HandleTypeDef *tim, uint8_t pixels) :
		timer(tim), numLeds(pixels), finalLed(pixels + 10), colors(
				new color32_t[pixels]) {

}

//helper for the timer channel assignment
uint32_t channelForTimer(TIM_HandleTypeDef *tim) {
	if (tim == &htim2)
		return TIM_CHANNEL_2;
	return TIM_CHANNEL_3;
}

void SK6812Chain::begin() {
	if (HAL_TIM_PWM_Start_DMA(timer, channelForTimer(timer), (uint32_t*) dmaBuf,
			DMA_BUF_LEDS * sizeof(color32_t) * 8) != HAL_OK) {
		Error_Handler();
	} else {
		currentLed = 0;
		dmaWorking = true;
		colorsChanged = false;
		resetCodeSent = false;
	}
}

void SK6812Chain::fillNextBuffer(uint16_t *buf) {
	constexpr uint8_t pixelsToSend = DMA_BUF_LEDS / 2; // such that we send half a buffer at a time
	if (currentLed >= numLeds) { // we need to send the reset code (low for 80us according to the datasheet)
		// just fill half the buffer like always even though it only needs to be down for
		memset(buf, 0x00, pixelsToSend * sizeof(color32_t) * 4 * sizeof(uint16_t));
		resetCodeSent = true;
	} else {
		uint8_t colorsSent = 0;
		while (colorsSent < pixelsToSend && currentLed < numLeds) {
			colorToPWM(buf, colors[currentLed]);
			// move the buf pointer forward by one LED
			buf = buf + (sizeof(color32_t) * 8);
			++currentLed;
			++colorsSent;
		}
	}
	currentLed %= finalLed;
}

void SK6812Chain::transmitComplete() {
	fillNextBuffer(&dmaBuf[DMA_BUF_LEDS * sizeof(color32_t) * 4]);
	if (resetCodeSent) {
		lastComplete = TickTimer_get();
		// stop the  circular-mode DMA until it's time for the next frame
		// if my math is right, without suspending it like this we would be updating
		// 1,600+ times a second, I've limited it to about 40 via PIXEL_UPDATE_MS
		if(!HAL_TIM_PWM_Stop_DMA(timer, channelForTimer(timer)) != HAL_OK){
			Error_Handler();
		}
		dmaWorking = false;
	}
}

void SK6812Chain::transmitHalfComplete() {
	fillNextBuffer(dmaBuf);
}

void SK6812Chain::tick() {
	if (dmaWorking)
		return;
	tick_t now = TickTimer_get();
	if (colorsChanged
			&& TickTimer_tickDistanceMs(lastComplete, now) >= PIXEL_UPDATE_MS) {
		// time to start the circular DMA
		begin();
	}

}

void SK6812Chain::setPixel(uint8_t px, color32_t col) {
	colorsChanged = (colors[px] != col);
	colors[px] = col;
}

//==================================================================

PixelProcessor::PixelProcessor() :
		mainPx(&htim2, 15), matrixPx(&htim5, 50) {

}


void PixelProcessor::begin(){
	mainPx.begin();
	matrixPx.begin();
}

void PixelProcessor::init(){
	// 1. set colors for the voice parameters
	updateForCutoff(CUTOFF_DEFAULT);
	updateForRes(RES_DEFAULT);
	updateForFold(FOLD_DEFAULT);
	updateForPWM(PWM_DEFAULT);
}

void PixelProcessor::txHalfComplete(TIM_HandleTypeDef* tim){
	if(tim == &htim2){
		mainPx.transmitHalfComplete();
	} else {
		matrixPx.transmitHalfComplete();
	}
}

void PixelProcessor::txComplete(TIM_HandleTypeDef* tim){
	if(tim == &htim2){
		mainPx.transmitComplete();
	} else {
		matrixPx.transmitComplete();
	}
}

void PixelProcessor::tick(){
	mainPx.tick();
	matrixPx.tick();
}

void PixelProcessor::setMainPixel(uint8_t id, color32_t color){
	mainPx.setPixel(id, color);
}

void PixelProcessor::setMatrixPixel(uint8_t src, uint8_t dest, color32_t color){
	src = src % 6;
	uint8_t idx = (src * 8) + dest;
	matrixPx.setPixel(idx, color);
}


void PixelProcessor::setSourceBankPixel(uint8_t right, color32_t color){
	uint8_t idx = (right > 0) ? 49 : 48;
	matrixPx.setPixel(idx, color);
}

color32_t PixelProcessor::getMainPixel(uint8_t id){
	return mainPx.getPixel(id);
}

color32_t PixelProcessor::getMatrixPixel(uint8_t src, uint8_t dest){
	src = src % 6;
	uint8_t idx = (src * 8) + dest;
	return matrixPx.getPixel(idx);
}

color32_t PixelProcessor::getSourceBankPixel(uint8_t right){
	uint8_t idx = (right > 0) ? 49 : 48;
	return matrixPx.getPixel(idx);
}

//Color Business=========================================================

void PixelProcessor::updateForCutoff(uint16_t val){
	static const color32_t minColor = color32_getWithBrightness(ColorID::AquaMarine, 160);
	static const color32_t maxColor = color32_getWithBrightness(ColorID::OrangeRed, 160);
	color32_t col = color32_blend16Bit(minColor, maxColor, val, 4095);
	setMainPixel(pixelID::pxCutoff, col);
}

void PixelProcessor::updateForRes(uint16_t val){
	static const color32_t minColor = color32_getWithBrightness(ColorID::AquaMarine, 160);
	static const color32_t maxColor = color32_getWithBrightness(ColorID::OrangeRed, 160);
	color32_t col = color32_blend16Bit(minColor, maxColor, val, 255);
	setMainPixel(pixelID::pxRes, col);
}

void PixelProcessor::updateForFold(uint16_t val){
	static const color32_t minColor = color32_getWithBrightness(ColorID::AquaMarine, 160);
	static const color32_t maxColor = color32_getWithBrightness(ColorID::OrangeRed, 160);
	color32_t col = color32_blend16Bit(minColor, maxColor, val, 4095);
	setMainPixel(pixelID::pxFold, col);
}

void PixelProcessor::updateForPWM(uint16_t val){
	static const color32_t minColor = color32_getWithBrightness(ColorID::AquaMarine, 160);
	static const color32_t maxColor = color32_getWithBrightness(ColorID::OrangeRed, 160);
	color32_t col = color32_blend16Bit(minColor, maxColor, val, 4095);
	setMainPixel(pixelID::pxPulseWidth, col);
}
 void PixelProcessor::updateForMod(mod_t m){
	 static const color32_t minColor = color32_getWithBrightness(ColorID::Teal, 160);
	 static const color32_t maxColor = color32_getWithBrightness(ColorID::LightPink, 160);
	 static const color32_t offColor = color32_getWithBrightness(ColorID::LightYellow, 85);
	 uint8_t src = get_mod_source(m);
	 uint8_t dest = get_mod_dest(m);
	 int8_t depth = get_mod_depth(m);
	 color32_t col;
	 if(depth == 0){
		 col = offColor;
	 } else if (depth > 0){
		 col = color32_blend16Bit(offColor, maxColor, (uint8_t)depth,  128);
	 } else {
		 col = color32_blend16Bit(offColor, minColor, (uint8_t)std::abs(depth), 128);
	 }
	 setMatrixPixel(src, dest, col);
 }
//==================================================================

pixel_processor_t create_pixel_processor(){
	return new PixelProcessor();
}

void tick_pixel_processor(pixel_processor_t proc){
	PixelProcessor* ptr = static_cast<PixelProcessor*>(proc);
	ptr->tick();
}

void init_pixels(pixel_processor_t proc){
	PixelProcessor* ptr = static_cast<PixelProcessor*>(proc);
	ptr->init();
}

void pixel_tx_complete(pixel_processor_t proc, TIM_HandleTypeDef* tim){
	PixelProcessor* ptr = static_cast<PixelProcessor*>(proc);
	ptr->txComplete(tim);
}

void pixel_tx_half_complete(pixel_processor_t proc, TIM_HandleTypeDef* tim){
	PixelProcessor* ptr = static_cast<PixelProcessor*>(proc);
	ptr->txHalfComplete(tim);
}

