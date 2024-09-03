/*
 * Color565.c
 *
 *  Created on: Sep 2, 2024
 *      Author: hayden
 */
#include "Color565.h"


color16_t color565_fromRGB(uint8_t r, uint8_t g, uint8_t b){
	float fR = (float)r / 255.0f;
	float fG = (float)g / 255.0f;
	float fB = (float)b / 255.0f;

	r = (uint8_t)fR * 32.0f;
	g = (uint8_t)fG * 64.0f;
	b = (uint8_t)fB * 32.0f;

	return (r << 11) | (g << 5) | b;
}


color24_t color565_to24Bit(color16_t col){

	color24_t out;

	uint8_t b = col & 0x0007;
	uint8_t g = col & 0x01F8;
	uint8_t r = col & 0x3E00;

	float fR = (float)r / 32.0f;
	float fG = (float)g / 64.0f;
	float fB = (float)b / 32.0f;

	out.r = (uint8_t)fR * 255.0f;
	out.g = (uint8_t)fG * 255.0f;
	out.b = (uint8_t)fB * 255.0f;

	return out;
}


