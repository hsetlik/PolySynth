/*
 * Graphics.cpp
 *
 *  Created on: Aug 22, 2024
 *      Author: hayden
 */

#include "Graphics.h"

GraphicsProcessor::GraphicsProcessor() :
		queue(static_cast<DisplayQueue*>(mainDispQueue)), dmaBusy(false) {
// initialize the screen
	ILI9341_Init();

}

GraphicsProcessor::~GraphicsProcessor() {

}

void GraphicsProcessor::initViews() {

	// envelope views
	env1View.setParams(&patch->envs[0]);
	env1View.setName("ADSR 1");
	env2View.setParams(&patch->envs[1]);
	env2View.setName("ADSR 2");

	// mixer views
	mix1View.setParams(&patch->oscs[0]);
	mix1View.setName("DCO 1 Mix");
	mix2View.setParams(&patch->oscs[1]);
	mix2View.setName("DCO 2 Mix");

	// LFO views
	lfo1View.setParams(&patch->lfos[0]);
	lfo1View.setName("LFO 1");
	lfo2View.setParams(&patch->lfos[1]);
	lfo2View.setName("LFO 2");
	lfo3View.setParams(&patch->lfos[2]);
	lfo3View.setName("LFO 3");

	// tuning view
	tuningView.setParams(&patch->oscs[0], &patch->oscs[1]);
	tuningView.setName("Tuning");

	// modal view
	modalView.setPatch(patch);

	views.push_back(&env1View);
	views.push_back(&env2View);
	views.push_back(&mix1View);
	views.push_back(&mix2View);
	views.push_back(&lfo1View);
	views.push_back(&lfo2View);
	views.push_back(&lfo3View);
	views.push_back(&tuningView);
	views.push_back(&modalView);

	// initialize all the views' child components
	for (View *v : views) {
		v->initChildren();
	}

	// make envelope 1 visible to start
	visibleView = &env1View;
	visibleView->draw();
}

void GraphicsProcessor::dmaFinished() {
	dmaBusy = false;
}

void GraphicsProcessor::checkGUIUpdates() {
	if (inModalMode && modalView.timeToRemove()) {
		undrawModal();
		inModalMode = false;
	}
	if (!dmaBusy && !queue->empty()) {
		runFront();
	}
}

bool GraphicsProcessor::needsLFOData(){
	if(needsRedrawChecks){
		return static_cast<LFOView*>(visibleView)->needsRedraw();
	} else
		return false;
}

void GraphicsProcessor::updateLFOs(uint16_t l1, uint16_t l2, uint16_t l3) {
	// update the data for all 3 LFOs
	lfo1View.updateGraph(l1);
	lfo2View.updateGraph(l2);
	lfo3View.updateGraph(l3);
	// only redraw the visible one
	visibleView->draw();
}



View* GraphicsProcessor::viewForID(uint8_t idx) {
	ViewID id = (ViewID) idx;
	switch (id) {
	case vEnv1:
		return &env1View;
	case vEnv2:
		return &env2View;
	case vLFO1:
		return &lfo1View;
	case vLFO2:
		return &lfo2View;
	case vLFO3:
		return &lfo3View;
	case vMix1:
		return &mix1View;
	case vMix2:
		return &mix2View;
	case vTune:
		return &tuningView;
	default:
		return nullptr;
	}
}

void GraphicsProcessor::selectView(uint8_t idx) {
	View *v = viewForID(idx);
	if (v != nullptr && v != visibleView) {
		visibleView = v;
		if (visibleView == &lfo1View || visibleView == &lfo2View
				|| visibleView == &lfo3View) {
			needsRedrawChecks = true;
		} else {
			needsRedrawChecks = false;
		}
		visibleView->draw();
	}

}
//----------------------------------------

void GraphicsProcessor::pushTask(DrawTask task) {
	queue->push(task);
}

void GraphicsProcessor::runFront() {
	DrawTask t = queue->popFront();
// run the callback to render the pixels
	t.func(t.area, dmaBuf);
	ILI9341_DrawImage_DMA(t.area.x, t.area.y, t.area.w, t.area.h, dmaBuf);
	dmaBusy = true;

}

void GraphicsProcessor::undrawModal() {
	auto chunks = modalView.getChunksForArea();
	// bc our visibleView* doesn't change when we enter modal mode
	visibleView->redrawChunks(chunks);
}

void GraphicsProcessor::paramUpdated(uint8_t param) {
	View *v = viewForParam(param);
	if (v != nullptr) {
		inModalMode = (v == &modalView);
		v->paramUpdated(param);
	}
}

View* GraphicsProcessor::viewForParam(uint8_t p) {
	// refer to the ParamID enum in Patch.h for my reasoning here
	if (p < ParamID::pEnv2Attack) { // envelopes--------------
		return &env1View;
	} else if (p >= ParamID::pEnv2Attack && p < ParamID::pLFO1Freq) {
		return &env2View;
	} else if (p >= ParamID::pLFO1Freq && p < ParamID::pLFO2Freq) {
		return &lfo1View;
	} else if (p >= ParamID::pLFO2Freq && p < ParamID::pLFO3Freq) {
		return &lfo2View;
	} else if (p >= ParamID::pLFO3Freq && p < ParamID::pOsc1Coarse) {
		return &lfo3View;
	} else if (p >= ParamID::pOsc1SquareLevel && p < ParamID::pOsc2Coarse) {
		return &mix1View;
	} else if (p >= ParamID::pOsc2SquareLevel && p < ParamID::pFilterCutoff) {
		return &mix2View;
	} else if ((p >= ParamID::pOsc1Coarse && p < ParamID::pOsc1PulseWidth)
			|| (p >= ParamID::pOsc2Coarse && p < ParamID::pOsc2PulseWidth)) {
		return &tuningView;
	} else if ((p == ParamID::pOsc1PulseWidth || p == ParamID::pOsc2PulseWidth)
			|| (p >= ParamID::pFilterCutoff)) {
		return &modalView;
	}
	return nullptr;
}
//======================================================

graphics_processor_t create_graphics_processor() {
	return new GraphicsProcessor();
}

void disp_dma_finished(graphics_processor_t proc) {
	GraphicsProcessor *ptr = static_cast<GraphicsProcessor*>(proc);
	ptr->dmaFinished();
}

