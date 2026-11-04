/*
 * Graphics.h
 *
 *  Created on: Aug 22, 2024
 *      Author: hayden
 */

#ifndef INC_GRAPHICS_H_
#define INC_GRAPHICS_H_
#include "Component.h"



/** this file's job is:
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
#include "Label.h"
#include "BarGraph.h"
#include "EnvView.h"
#include "OscView.h"
#include "LFOView.h"
#include "ModalView.h"
//--------------------
//Patch Save/Load interface==================




//==================================================

// enum of the view types
enum ViewID {
	vEnv1,
	vEnv2,
	vLFO1,
	vLFO2,
	vLFO3,
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
	LFOView lfo1View;
	LFOView lfo2View;
	LFOView lfo3View;
	OscTuningView tuningView;
	ModalChangeView modalView;
	std::vector<View*> views;
	bool inModalMode = false;
	bool needsRedrawChecks = false;
	void checkForRedraw();
	// helper to return the relevant view for a given parameter change
	View* viewForParam(uint8_t p);
	// modal helper
	void undrawModal();
	View* viewForID(uint8_t id);
	// LFO graphing stuff
	tick_t lastLFOUpdate = 0;
public:
	GraphicsProcessor();
	~GraphicsProcessor();
	void dmaFinished();
	void checkGUIUpdates();
	bool needsLFOData();

	void updateLFOs(uint16_t l1, uint16_t l2, uint16_t l3);
	void setPatchData(patch_t* p) {
		patch = p;
	}
	// call this to allocate our view objects AFTER getting the patch data
	void initViews();
	// call this from the synth processor
	void paramUpdated(uint8_t param);
	// callback for selecting a new view and drawing it
	void selectView(uint8_t  id);

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



#undef EXTERNC

#endif /* INC_GRAPHICS_H_ */
