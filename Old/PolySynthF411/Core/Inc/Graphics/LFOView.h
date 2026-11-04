/*
 * LFOView.h
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */

#ifndef INC_GRAPHICS_LFOVIEW_H_
#define INC_GRAPHICS_LFOVIEW_H_
#include "Component.h"

#ifdef __cplusplus
#include "Label.h"
#include "BarGraph.h"
#define LFO_GRAPH_POINTS 100
// basic single-ended ring buffer
class GraphBuffer {
private:
	uint16_t data[LFO_GRAPH_POINTS];
	uint8_t head = 0;
public:
	GraphBuffer();
	void push(uint16_t val);
	uint16_t operator[](uint8_t idx){
		return data[(head + idx) % LFO_GRAPH_POINTS];
	}
};

class LFOGraph : public Component {
private:
	enum DrawState {
		ReadyToDraw,
		Drawing,
		FinishedDrawing
	};
	// state stuff
	tick_t lastGraphPoint = 0;

	// graph stuff
	GraphBuffer lfoPoints;
	line_t graphLines[LFO_GRAPH_POINTS - 1];
public:
	color16_t bkgndColor = color565_getColor16(ColorID::Black);
	color16_t lineColor = color565_getColor16(ColorID::LimeGreen);
	LFOGraph();

	bool needsGraphPoint(float freq);
	void addGraphPoint(uint16_t val);
	void updateGraphLines();
	void drawChunk(area_t chunk, uint16_t *buf) override;

};

class LFOView : public View {
private:
	lfo_t* params;
	// components
	Label lName;

	Label lTypeName;
	Label lTypeVal;

	Label lFreqName;
	Label lFreqVal;

	LFOGraph graph;

public:
	LFOView();
	void setParams(lfo_t* p){
		params = p;
	}
	void initChildren() override;
	void paramUpdated(uint8_t id) override;
	bool needsRedraw() {
		return graph.needsGraphPoint(params->freq);
	}
	void updateGraph(uint16_t val){
		graph.addGraphPoint(val);
	}
};



#endif

#ifdef __cplusplus
#define EXTERNC extern "c"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_GRAPHICS_LFOVIEW_H_ */
