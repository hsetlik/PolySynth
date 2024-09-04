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
#include <stdbool.h>
#include "Color565.h"
#include "Patch.h"
#define MAX_CHUNK_WIDTH 64
#define MAX_CHUNK_HEIGHT 64
#define MAX_CHUNK_PX 4096
#define MAX_CHUNK_BYTES 8192

// some C stuff
#ifdef __cplusplus
extern "C" {
#endif
area_t getOverlap(area_t a, area_t b);
bool hasOverlap(area_t a, area_t b);
bool pointInArea(area_t a, uint16_t x, uint16_t y);

uint16_t numChunksNeeded(area_t area);

// line stuff
typedef struct {
	point_t a;
	point_t b;
} line_t;

// pixel access
uint16_t* getPixel(uint16_t *buf, uint16_t width, uint16_t height, uint16_t x,
		uint16_t y);
uint16_t* pixelInChunk(uint16_t buf, area_t area);

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
#include <string>
#include <vector>

// each GUI element will be split up into some number of DrawTasks

typedef std::function<void(area_t a, uint16_t*)> DrawFunc;

struct DrawTask {
	DrawFunc func;
	area_t area;
};

#define DRAW_QUEUE_SIZE 35

// a basic circular buffer class to keep our DrawTasks in
template<typename T>
class RingBuffer {
private:
	std::unique_ptr<T[]> data;
	uint16_t head = 0;
	uint16_t tail = 0;
	bool isFull = false;
public:
	RingBuffer() :
			data(std::unique_ptr<T[]>(new T[DRAW_QUEUE_SIZE])) {

	}
	// add/remove elements
	T getFront() {
		if (empty()) {
			return T();
		}
		T value = data[tail];
		isFull = false;
		tail = (tail + 1) % DRAW_QUEUE_SIZE;
		return value;
	}
	void push(T obj) {
		data[head] = obj;
		if (isFull) {
			tail = (tail + 1) % DRAW_QUEUE_SIZE;
		}
		head = (head + 1) % DRAW_QUEUE_SIZE;
		isFull = (head == tail);
	}
	// check state
	bool full() {
		return isFull;
	}

	bool empty() {
		return (!isFull && (head == tail));
	}
};

// COMPONENT CLASSES ==================================
class Component {
protected:
	area_t area;
	uint8_t zIndex;
	// this splits the component up into task-sized chunks
	std::vector<area_t> getTaskChunks();
public:
	Component();
	virtual ~Component();
	void setArea(area_t a);
	void setZIndex(uint8_t val);
	// this needs to be overridden by all subclasses
	virtual void drawChunk(area_t chunk, uint16_t *buf)=0;
	// this can be called from the processor to draw all the chunks of this component
	void draw(RingBuffer<DrawTask> &queue);
};


//--------------------
class Label: public Component {
private:
	std::string text;
	FontDef *font;

	// drawing helper
	area_t getStringArea();
	// colors
	color16_t txtColor = color565_getColor16(ColorID::White);
	color16_t bkgndColor = color565_getColor16(ColorID::Black);
public:
	Label();
	Label(const std::string &s);
	void setFont(FontDef *f);
	void setText(const std::string& str);
	uint16_t getIdealWidth(uint16_t margin);
	uint16_t getIdealHeight(uint16_t margin);
	void drawChunk(area_t chunk, uint16_t *buf) override;
};

// keep track of all the labeled parameters in all the UI screens
enum LabelID {
	AttackMs,
	DecayMs,
	SustainPercent,
	ReleaseMs
};

//---------------------------------

class EnvGraph: public Component {
private:
	adsr_t* params;
public:
	color16_t lineColor = color565_getColor16(ColorID::Black);
	color16_t bkgndColor = color565_getColor16(ColorID::White);
	EnvGraph();
	void setParams(adsr_t* params);
	void drawChunk(area_t chunk, uint16_t *buf) override;
};


// VIEW==============================================

class View {
protected:
	std::vector<Component*> children;
public:
	View();
	virtual ~View();
	// override this to set the child components' positions and add the pointers to our children vector
	virtual void initChildren()=0;
	virtual void paramUpdated(uint8_t labelId){
	}
	void draw(RingBuffer<DrawTask>& queue);
};

// envelope view
class EnvView : public View {
private:
	adsr_t* params;
	Label aLabel;
	Label dLabel;
	Label sLabel;
	Label rLabel;
	EnvGraph graph;
public:
	EnvView();
	void setParams(adsr_t* p);
	void initChildren() override;
	void paramUpdated(uint8_t l) override;
private:
	void setLabels();
	std::string textForLabel(Label* l);
};

//==================================================
class GraphicsProcessor {
private:
	bool dmaBusy;
	uint16_t dmaBuf[MAX_CHUNK_PX];

	// queue stuff
	RingBuffer<DrawTask> queue;

	void pushTask(DrawTask task);
	void runFront();

public:
	GraphicsProcessor();
	void dmaFinished();
	void checkDrawQueue();

};

#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/
typedef void *graphics_processor_t;

EXTERNC graphics_processor_t create_graphics_processor();
EXTERNC void disp_dma_finished(graphics_processor_t proc);
EXTERNC void check_draw_queue(graphics_processor_t proc);

#undef EXTERNC

#endif /* INC_GRAPHICS_H_ */
