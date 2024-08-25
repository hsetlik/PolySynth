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
#define MAX_CHUNK_WIDTH 64
#define MAX_CHUNK_HEIGHT 64
#define MAX_CHUNK_PX 4096
#define MAX_CHUNK_BYTES 8192

// some C stuff
#ifdef __cplusplus
extern "C"{
#endif

area_t getOverlap(area_t a, area_t b);

uint16_t numChunksNeeded(area_t area);

#ifdef __cplusplus
}
#endif

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
 * Because a 150kb screen buffer can't fit in our 128kb of RAM,
 * we'll split our graphics draw calls into chunks of 64x64 max
 * and have a DMA buffer of 8192 bytes (4096 pixels)
 *
 *	 */


#ifdef __cplusplus
#include <functional>
#include <memory>


// each GUI element will be split up into some number of DrawTasks

typedef std::function<void(area_t a, uint16_t*)> DrawFunc;




struct DrawTask {
	DrawFunc func;
	area_t area;
};

#define DRAW_QUEUE_SIZE 35

// a basic circular buffer class to keep our DrawTasks in
template <typename T>
class RingBuffer{
private:
	std::unique_ptr<T[]> data;
	uint16_t head = 0;
	uint16_t tail = 0;
	bool isFull = false;
public:
	RingBuffer() : data(std::unique_ptr<T[]>(new T[DRAW_QUEUE_SIZE])){

	}
	// add/remove elements
	T getFront(){
		if(empty()){
			return T();
		}
		T value = data[tail];
		isFull = false;
		tail = (tail + 1) % DRAW_QUEUE_SIZE;
		return value;
	}
	void push(T obj){
		data[head] = obj;
		if(isFull){
			tail = (tail + 1) % DRAW_QUEUE_SIZE;
		}
		head = (head + 1) % DRAW_QUEUE_SIZE;
		isFull = (head == tail);
	}
	// check state
	bool full(){
		return isFull;
	}

	bool empty(){
		return (!isFull && (head == tail));
	}
};

//==================================================





class GraphicsProcessor{
private:
	bool dmaBusy;
	uint16_t dmaBuf[MAX_CHUNK_PX];

	// queue stuff
	DrawTask drawQueue[DRAW_QUEUE_SIZE];
	uint8_t front = 0;
	uint8_t length = 0;
	RingBuffer<DrawTask> queue;

	void pushTask(DrawTask task);
	void runFront();

	// helpers for drawing
	void drawLabel(area_t area, const char* text, uint16_t bkgndColor, uint16_t textColor);
public:
	GraphicsProcessor();
	void dmaFinished();

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
