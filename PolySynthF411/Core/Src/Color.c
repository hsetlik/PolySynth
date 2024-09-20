/*
 * Color565.c
 *
 *  Created on: Sep 2, 2024
 *      Author: hayden
 */
#include <Color.h>


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

	uint16_t b = col & 0x0007;
	uint16_t g = (col & 0x01F8) >> 6;
	uint16_t r = (col & 0x3E00) >> 11;

	float fR = (float)r / 32.0f;
	float fG = (float)g / 64.0f;
	float fB = (float)b / 32.0f;

	out.r = (uint8_t)fR * 255.0f;
	out.g = (uint8_t)fG * 255.0f;
	out.b = (uint8_t)fB * 255.0f;

	return out;
}

//=====================================================

const color24_t colors24[] = {
		{0, 0, 0}, // black
		{255, 255, 255}, // white
		{128, 0, 0}, // maroon
		{250, 128, 114}, // salmon
		{255, 69, 0}, // orange red
		{255, 140, 0}, // dark orange
		{255, 215, 0}, // gold
		{255, 255, 0}, // yellow
		{50, 205, 50}, // lime green
		{0, 100, 0}, // dark green
		{143, 188, 143}, // dark sea green
		{47, 79, 79}, // dark slate gray
		{0, 128, 128}, // teal
		{0, 255, 255}, // cyan
		{64, 224, 208}, // turquoise
		{127, 255, 212}, // aqua marine
		{0, 191, 255}, // deep sky blue
		{75, 0, 130}, // indigo
		{186, 85, 211}, // medium orchid
		{255, 0, 255}, // magenta
		{255, 20, 147}, // deep pink
		{255, 182, 193}, // light pink
		{255, 228, 196}, // bisque
		{255, 255, 224}, // light yellow
		{160, 82, 45}, // sienna
		{244, 164, 96}, // sandy brown
		{188, 143, 143}, // rosy brown
		{255, 228, 225}, // misty rose
		{245, 255, 250}, // mint cream
		{112, 128, 124}, // slate gray
		{176, 196, 202}, // light steel blue
		{240, 248, 255}, // alice blue
		{240, 255, 240}, // honeydew
};


color16_t color565_getColor16(uint8_t id){
	color24_t col = colors24[id];
	return color565_fromRGB(col.r, col.g, col.b);
}


color24_t color24_lerp16(color24_t a, color24_t b, uint16_t t, uint16_t max){
	color24_t col;
	float fT = (float)t / (float)max;
	col.r = (uint8_t)((float)b.r * fT) + (uint8_t)((float)a.r * (1.0f - fT));
	col.g = (uint8_t)((float)b.g * fT) + (uint8_t)((float)a.g * (1.0f - fT));
	col.b = (uint8_t)((float)b.b * fT) + (uint8_t)((float)a.b * (1.0f - fT));
	return col;
}


//=====================================================

color32_t color32_getFullBrightness(uint8_t id){
	return color32_getWithBrightness(id, 255);
}

color32_t color32_getWithBrightness(uint8_t id, uint32_t b){
		color24_t c24 = colors24[id];
	color32_t col = 0;
	col = (c24.r << 24) | (c24.g << 16) | (c24.b << 8) | b;
	return col;
}

