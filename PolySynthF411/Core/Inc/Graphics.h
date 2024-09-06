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
#include "DisplayQueue.h"
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


// COMPONENT CLASSES ==================================
class Component {
protected:
	DisplayQueue* const queue;
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
	void draw();
};


//--------------------
class Label: public Component {
protected:
	std::string text;
	FontDef *font;
	bool needsRedraw = true;

	// drawing helper
	area_t getStringArea();
public:
	// colors
	color16_t txtColor = color565_getColor16(ColorID::White);
	color16_t bkgndColor = color565_getColor16(ColorID::Black);
	Label();
	Label(const std::string &s);
	void setFont(FontDef *f);
	void setText(const std::string& str);
	uint16_t getIdealWidth(uint16_t margin);
	uint16_t getIdealHeight(uint16_t margin);
	void drawChunk(area_t chunk, uint16_t *buf) override;
	bool textHasChanged() { return needsRedraw; }
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
	std::string viewName = "Null View";
public:
	View();
	virtual ~View();
	void setName(const std::string& str);
	// override this to set the child components' positions and add the pointers to our children vector
	virtual void initChildren()=0;
	virtual void paramUpdated(uint8_t id){
	}
	void draw();
};

// envelope view ---------------------------
class EnvView : public View {
private:
	adsr_t* params;
	Label nameLabel;
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

// mixer view (and associated components)-------------------------------

class BarGraph : public Component {
private:
	const uint16_t maxLevel;
	uint16_t currentLevel = 0;
	uint16_t margin = 3;
	area_t getBarArea();
public:
	color16_t barColor = color565_getColor16(ColorID::Maroon);
	color16_t bkgndColor = color565_getColor16(ColorID::Salmon);
	BarGraph(uint16_t maxValue=255);
	void setLevel(uint16_t lvl){
		currentLevel = lvl;
	}
	void drawChunk(area_t chunk, uint16_t *buf) override;
};

class MixerView : public View {
private:
	dco_t* params;

	Label nameLabel;

	Label sawNameLabel;
	Label sawLevelLabel;
	BarGraph sawLevelGraph;

	Label triNameLabel;
	Label triLevelLabel;
	BarGraph triLevelGraph;

	Label pulseNameLabel;
	Label pulseLevelLabel;
	BarGraph pulseLevelGraph;

	Label masterNameLabel;
	Label masterLevelLabel;
	BarGraph masterLevelGraph;

public:
	MixerView();
	void setParams(dco_t* p){
		params = p;
	}
	void initChildren() override;
	void paramUpdated(uint8_t id) override;
private:


};

//TUNING VIEW-------------------------------------

class BipolarBarGraph : public Component {
private:
	const int16_t maxLevel;
	int16_t currentLevel = 0;
	uint16_t margin = 3;
	area_t getBarArea();
public:
	color16_t negBarColor = color565_getColor16(ColorID::Maroon);
	color16_t posBarColor = color565_getColor16(ColorID::DarkGreen);
	color16_t bkgndColor = color565_getColor16(ColorID::Salmon);
	BipolarBarGraph(int16_t max=127);
	void drawChunk(area_t chunk, uint16_t *buf) override;
	void setLevel(int16_t lvl) {
		currentLevel = lvl;
	}
};

class OscTuningView : public View {
private:

	//state
	dco_t* params1;
	dco_t* params2;

	//components
	Label nameLabel;
	//osc1
	Label lOsc1;
	Label lFineName1;
	Label lFineVal1;
	BipolarBarGraph gFine1;
	Label lCoarseName1;
	Label lCoarseVal1;
	BipolarBarGraph gCoarse1;
	//osc2
	Label lOsc2;
	Label lFineName2;
	Label lFineVal2;
	BipolarBarGraph gFine2;
	Label lCoarseName2;
	Label lCoarseVal2;
	BipolarBarGraph gCoarse2;
public:
	OscTuningView();
	void setParams(dco_t* osc1, dco_t* osc2){
		params1 = osc1;
		params2 = osc2;
	}
	void initChildren() override;
	void paramUpdated(uint8_t id) override;
};




//==================================================

// enum of the view types
enum ViewID {
	vEnv1,
	vEnv2,
	vMix1,
	vMix2,
	vTune
};


class GraphicsProcessor {
private:
	DisplayQueue* const queue;
	patch_t* patch = nullptr;
	bool dmaBusy;
	uint16_t dmaBuf[MAX_CHUNK_PX];
	void pushTask(DrawTask task);
	void runFront();
	// keep our views here
	View* visibleView = nullptr;
	EnvView env1View;
	EnvView env2View;
	MixerView mix1View;
	MixerView mix2View;
	OscTuningView tuningView;
	std::vector<View*> views;
	// helper to return the relevant view for a given parameter change
	View* viewForParam(uint8_t p);
public:
	GraphicsProcessor();
	~GraphicsProcessor();
	void dmaFinished();
	void checkDrawQueue();
	void setPatchData(patch_t* p) {
		patch = p;
	}
	// call this to allocate our view objects AFTER getting the patch data
	void initViews();
	// call this from the synth processor
	void paramUpdated(uint8_t param);

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
