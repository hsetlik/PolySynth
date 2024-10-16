/*
 * LFOView.cpp
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */
#include "LFOView.h"
// graph
GraphBuffer::GraphBuffer() {

}

void GraphBuffer::push(uint16_t val) {
	data[head] = val;
	head = (head + 1) % LFO_GRAPH_POINTS;
}

LFOGraph::LFOGraph() {

}

void LFOGraph::updateGraphLines() {
	int16_t x0 = (int16_t) area.x;
	const int16_t dX = (int16_t) ((float) area.w / (float) LFO_GRAPH_POINTS);
	const float yScale = (float) area.h / 4096.0f;
	const int16_t yBottom = (int16_t) area.y + area.h;
	int16_t y0 = yBottom - (int16_t) ((float) lfoPoints[0] * yScale);
	int16_t x1, y1;
	for (uint8_t l = 1; l < LFO_GRAPH_POINTS; l++) {
		x1 = x0 + dX;
		y1 = yBottom - (int16_t) ((float) lfoPoints[l] * yScale);
		graphLines[l - 1] = { { x0, y0 }, { x1, y1 } };
		x0 = x1;
		y0 = y1;
	}
}

bool LFOGraph::needsGraphPoint(float freq) {
	static tick_t now = 0;
	now = TickTimer_get();
	// check if the frequency has changed
	static float prevFreq = 1.0f;
	static float periodMs = 1000.0f / prevFreq;
	if (prevFreq != freq) {
		prevFreq = freq;
		periodMs = 1000.0f / prevFreq;
	}
	return TickTimer_tickDistanceMs(lastGraphPoint, now) > periodMs;
}

void LFOGraph::addGraphPoint(uint16_t val) {
	lfoPoints.push(val);
	lastGraphPoint = TickTimer_get();
	updateGraphLines();
}

void LFOGraph::drawChunk(area_t chunk, uint16_t *buf) {
	// 1: fill with the background color
	uint16_t *px = nullptr;
	for (uint16_t x = 0; x < chunk.w; x++) {
		for (uint16_t y = 0; y < chunk.h; y++) {
			px = getPixel(buf, chunk.w, chunk.h, x, y);
			*px = (uint16_t) bkgndColor;
		}
	}
	// 2: draw any necessary lines
	bool startedDrawing = false;
	for (uint8_t l = 0; l < LFO_GRAPH_POINTS - 1; l++) {
		if (isLineInChunk(graphLines[l], chunk)) {
			startedDrawing = true;
			drawLineInChunk(graphLines[l], chunk, buf, lineColor);
		} else if (startedDrawing) {
			// if we've exited the group of lines that this chunk intersects, we're done
			return;
		}
	}
}

// view----------------

LFOView::LFOView() {

}

std::string LFOTypeStr(uint8_t id) {
	switch (id) {
	case LFOType::Sine:
		return "Sine";
	case LFOType::Triangle:
		return "Triangle";
	case LFOType::Ramp:
		return "Ramp";
	case LFOType::Perlin:
		return "Perlin";
	default:
		return "null";
	}
}

std::string freqString5Digit(float freq) {
	return std::to_string(freq).substr(0, 4) + " hZ";
}

void LFOView::initChildren() {

	// name
	const uint16_t nameHeight = 24;
	area_t nameArea;
	nameArea.x = 0;
	nameArea.y = 0;
	nameArea.w = ILI9341_WIDTH;
	nameArea.h = nameHeight;

	lName.setArea(nameArea);
	lName.setFont(&Font_11x18);
	lName.setText(viewName);

	// LFO type
	const uint16_t labelWidth = ILI9341_WIDTH / 4;
	const uint16_t labelHeight = 14;
	area_t typeArea;
	typeArea.x = 0;
	typeArea.y = ILI9341_HEIGHT - (2 * labelHeight);
	typeArea.w = labelWidth;
	typeArea.h = labelHeight;
	lTypeName.setArea(typeArea);
	lTypeName.setText("LFO Type");

	typeArea.y += labelHeight;
	lTypeVal.setArea(typeArea);
	lTypeVal.setText(LFOTypeStr(params->lfoType));

	// frequency
	area_t freqArea;
	freqArea.x = labelWidth;
	freqArea.y = ILI9341_HEIGHT - (2 * labelHeight);
	freqArea.w = labelWidth;
	freqArea.h = labelHeight;
	lFreqName.setArea(freqArea);
	lFreqName.setText("Frequency");
	freqArea.y += labelHeight;

	lFreqVal.setArea(freqArea);
	lFreqVal.setText(freqString5Digit(params->freq));

	area_t graphArea;
	graphArea.x = 0;
	graphArea.y = nameHeight;
	graphArea.w = ILI9341_WIDTH;
	graphArea.h = ILI9341_HEIGHT - (nameHeight + (2 * labelHeight));
	graph.setArea(graphArea);

	children.push_back(&lName);
	children.push_back(&lTypeName);
	children.push_back(&lTypeVal);
	children.push_back(&lFreqName);
	children.push_back(&lFreqVal);
	children.push_back(&graph);

}

void LFOView::paramUpdated(uint8_t id) {
	uint8_t freqIdx = ParamID::pLFO1Freq;
	if (id >= ParamID::pLFO2Freq && id < ParamID::pLFO3Freq) {
		freqIdx = ParamID::pLFO2Freq;
	} else if (id >= ParamID::pLFO3Freq && id < ParamID::pOsc1Coarse) {
		freqIdx = ParamID::pLFO3Freq;
	}
	id = id - freqIdx;
	switch (id) {
	case 0: // freq
		lFreqVal.setText(freqString5Digit(params->freq));
		lFreqVal.draw();
		break;
	case 1: // mode
		lTypeVal.setText(LFOTypeStr(params->lfoType));
		lFreqVal.draw();
		break;
	default:
		break;
	}
}






