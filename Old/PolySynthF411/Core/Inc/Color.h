/*
 * Color565.h
 *
 *  Created on: Sep 2, 2024
 *      Author: hayden
 */

#ifndef INC_COLOR_H_
#define INC_COLOR_H_
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

// some typedefs for our colors
typedef uint16_t color16_t;
typedef uint32_t color32_t;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} color24_t;

color24_t color24_lerp16(color24_t a, color24_t b, uint16_t val, uint16_t max);

// converts from regular 24-bit RGB to packed 565 RGB
color16_t color565_fromRGB(uint8_t r, uint8_t g, uint8_t b);
color24_t color565_to24Bit(color16_t col);

typedef enum {
	Black,
	White,
	Maroon,
	Salmon,
	OrangeRed,
	DarkOrange,
	Gold,
	Yellow,
	LimeGreen,
	DarkGreen,
	DarkSeaGreen,
	DarkSlateGray,
	Teal,
	Cyan,
	Turquoise,
	AquaMarine,
	DeepSkyBlue,
	Indigo,
	MediumOrchid,
	Magenta,
	DeepPink,
	LightPink,
	Bisque,
	LightYellow,
	Sienna,
	SandyBrown,
	RosyBrown,
	MistyRose,
	MintCream,
	SlateGray,
	LightSteelBlue,
	AliceBlue,
	Honeydew

} ColorID;

color16_t color565_getColor16(uint8_t id);

//NEOPIXEL RGBW STUFF====================================
color32_t color32_getFullBrightness(uint8_t id);
color32_t color32_getWithBrightness(uint8_t id, uint32_t b);
color32_t color32_blend16Bit(color32_t a, color32_t b, uint16_t val, uint16_t max);

#ifdef __cplusplus
}
#endif

#endif /* INC_COLOR_H_ */
