/*
 * Color565.h
 *
 *  Created on: Sep 2, 2024
 *      Author: hayden
 */

#ifndef INC_COLOR565_H_
#define INC_COLOR565_H_
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

// some typedefs for our colors
typedef uint16_t color16_t;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} color24_t;


// converts from regular 24-bit RGB to packed 565 RGB
color16_t color565_fromRGB(uint8_t r, uint8_t g, uint8_t b);
color24_t color565_to24Bit(color16_t col);

typedef enum {
	Black,
	White
} ColorID;

color16_t color565_getColor16(uint8_t id);

#ifdef __cplusplus
}
#endif

#endif /* INC_COLOR565_H_ */
