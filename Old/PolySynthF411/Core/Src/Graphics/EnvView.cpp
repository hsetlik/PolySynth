/*
 * EnvView.cpp
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */
#include "EnvView.h"

// ADSR graph===========================================

float totalLengthMs(adsr_t *env) {
	return env->attack + env->decay + env->release;
}

uint16_t flerp16(uint16_t a, uint16_t b, float t) {
	return a + (uint16_t) ((float) (b - a) * t);
}

// define how short/long the flat section representing sustain can be
// depending on the length of the envelope
#define SUSTAIN_WIDTH_MIN 0.16f
#define SUSTAIN_WIDTH_MAX 0.84f
/** This works like:
 * 	0- colinear
 * 	1- clockwise
 * 	2- counterclockwise
 *
 */



// line math stuff----------------
// get the points for a given envelope within a given area
std::vector<point_t> getEnvPoints(adsr_t *env, area_t area) {
	std::vector<point_t> points;
	// first point is easy enough, just start from the bottom left corner
	points.push_back( { (int16_t) area.x, (int16_t) (area.y + area.h) });
	// figure out how much we should scale stuff relative to the length of the envelope
	const float envMs = totalLengthMs(env);
	float fLength = (float) (envMs - ENV_LENGTH_MIN)
			/ (float) (ENV_LENGTH_MAX - ENV_LENGTH_MIN);
	const int16_t slopedLengthMin = (int16_t) ((1.0f - SUSTAIN_WIDTH_MAX)
			* (float) area.w);
	const int16_t slopedLengthMax = (int16_t) ((1.0f - SUSTAIN_WIDTH_MIN)
			* (float) area.w);
	const int16_t totalSlopedLength = flerp16(slopedLengthMin, slopedLengthMax,
			fLength);
	const int16_t sustainLength = area.w - totalSlopedLength;

	int16_t attackX = (int16_t) ((env->attack / envMs)
			* (float) totalSlopedLength) + area.x;
	// add the top attack point 3px below the top of the component
	const int16_t topMargin = 3;
	points.push_back( { attackX, (int16_t) (area.y + topMargin) });
	// find the y coordinate for the sustain  value
	const int16_t sustainY = (int16_t) flerp16(area.y + topMargin,
			area.y + area.h, 1.0f - env->sustain);
	// find the x for the decay length
	int16_t decayX = attackX
			+ (int16_t) ((env->decay / envMs) * (float) totalSlopedLength);
	points.push_back( { decayX, sustainY });
	int16_t sustainEndX = decayX + sustainLength;
	points.push_back( { sustainEndX, sustainY });

	points.push_back(
			{ (int16_t) (area.x + area.w), (int16_t) (area.y + area.h) });
	return points;
}

std::vector<line_t> getEnvLines(adsr_t *env, area_t area) {
	std::vector<line_t> lines;
	std::vector<point_t> points = getEnvPoints(env, area);
	for (uint8_t i = 1; i < points.size(); i++) {
		lines.push_back( { points[i - 1], points[i] });
	}
	return lines;
}

//------
EnvGraph::EnvGraph() {

}

void EnvGraph::setParams(adsr_t *p) {
	params = p;
}

void EnvGraph::drawChunk(area_t chunk, uint16_t *buf) {
	std::vector<line_t> lines = getEnvLines(params, area);
	fillChunk(buf, chunk.w, chunk.h, bkgndColor);
	for (auto &line : lines) {
		if (isLineInChunk(line, chunk)) {
			drawLineInChunk(line, chunk, buf, lineColor);
		}
	}

}


EnvView::EnvView() {

}

void EnvView::setParams(adsr_t *p) {
	params = p;
}

void EnvView::initChildren() {
	// 1. set areas for the child components
	const uint16_t labelWidth = ILI9341_WIDTH / 4;
	const uint16_t labelHeight = 15;
	uint16_t lX = 0;
	uint16_t lY = ILI9341_HEIGHT - labelHeight;
	//set the labels
	aLabel.setArea( { lX, lY, labelWidth, labelHeight });
	lX += labelWidth;
	dLabel.setArea( { lX, lY, labelWidth, labelHeight });
	lX += labelWidth;
	sLabel.setArea( { lX, lY, labelWidth, labelHeight });
	lX += labelWidth;
	rLabel.setArea( { lX, lY, labelWidth, labelHeight });

	// set the name label
	const uint16_t nameHeight = 24; // for 11x18 font
	nameLabel.setFont(&Font_11x18);
	nameLabel.setText(viewName);
	nameLabel.setArea( { 0, 0, ILI9341_WIDTH, nameHeight });

	// set the graph
	graph.setArea(
			{ 0, nameHeight, ILI9341_WIDTH, (uint16_t) (lY - nameHeight) });

	// set the label text
	setLabels();

	//2. add everything to the children array
	children.push_back(&aLabel);
	children.push_back(&dLabel);
	children.push_back(&sLabel);
	children.push_back(&rLabel);
	children.push_back(&nameLabel);
	children.push_back(&graph);

}

void EnvView::setLabels() {

	aLabel.setText(textForLabel(&aLabel));
	dLabel.setText(textForLabel(&dLabel));
	sLabel.setText(textForLabel(&sLabel));
	rLabel.setText(textForLabel(&rLabel));

}

std::string EnvView::textForLabel(Label *l) {

	std::string full = "";
	if (l == &aLabel)
		full = "A: " + std::to_string(params->attack);
	else if (l == &dLabel)
		full = "D: " + std::to_string(params->decay);
	else if (l == &sLabel)
		full = "S: " + std::to_string((uint8_t) (params->sustain * 100.0f));
	else
		full = "R: " + std::to_string(params->release);

	// now we need to shorten the strings and add a suffix for each parameter
	if (l == &sLabel) {
		return full.substr(0, 5) + "\%";
	} else {
		return full.substr(0, 6) + "ms";
	}
}

void EnvView::paramUpdated(uint8_t l) {
	uint8_t attackIdx = (uint8_t) ParamID::pEnv1Attack;
	// figure out which envelope this is
	if (l > ParamID::pEnv1Release)
		attackIdx = (uint8_t) ParamID::pEnv2Attack;
	// see the ParamID enum in Patch.h for why this code makes sense
	l = l - attackIdx;
	Label *changedLabel = nullptr;
	switch (l) {
	case 0: // attack
		aLabel.setText(textForLabel(&aLabel));
		changedLabel = &aLabel;
		break;
	case 1: // decay
		dLabel.setText(textForLabel(&dLabel));
		changedLabel = &dLabel;
		break;
	case 2: // sutain
		sLabel.setText(textForLabel(&sLabel));
		changedLabel = &sLabel;
		break;
	case 3: // release
		rLabel.setText(textForLabel(&rLabel));
		changedLabel = &rLabel;
		break;
	default:
		break;
	}

	/** This makes it so that we only redraw the label and the env graph when
	 * a value changes enough to alter the text of the label, the idea being
	 * that a change so small it isn't visible with however many decimal points
	 * on a label is probably also so small that the envelope graph won't be significantly
	 * different and updating it is a waste of time.
	 */

	if (changedLabel->textHasChanged()) {
		changedLabel->draw();
		graph.draw();
	}

}






