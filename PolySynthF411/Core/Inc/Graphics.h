/*
 * Graphics.h
 *
 *  Created on: Aug 22, 2024
 *      Author: hayden
 */

#ifndef INC_GRAPHICS_H_
#define INC_GRAPHICS_H_
#include "main.h"
#include "ili9341.h"

/** this guy's job is:
 * 1. recieve events from the SynthProcessor (components?)
 * 2. convert those events to a set of pixels to draw
 * 3. send that pixel data over SPI DMA
 *
 * each pixel is 16 bits so to redraw the whole screen,
 * we need to transmit a total of:
 * - 153,600 bytes
 * - 1,228,800 bits (SPI clock cycles)
 *
 * With a prescaler of 2 (lowest option)
 * our SPI bit clock is at 24 MHz, so
 * drawing the entire frame should
 * take ~19.5ms
 *
 *
 *
 *	 */


#ifdef __cplusplus

class GraphicsProcessor{
private:
	bool dmaBusy;
public:
	GraphicsProcessor();
	void dmaFinished() {
		dmaBusy = false;
	}

};


#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/
typedef void* graphics_processor_t;

EXTERNC graphics_processor_t create_graphics_processor();
EXTERNC void disp_dma_finished(graphics_processor_t proc);

#undef EXTERNC

#endif /* INC_GRAPHICS_H_ */
