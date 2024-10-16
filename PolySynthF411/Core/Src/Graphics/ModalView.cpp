/*
 * ModalView.cpp
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */

#include "ModalView.h"
// base comp----------------------------------------
BaseChangeComponent::BaseChangeComponent() {
	lName.bkgndColor = bkgndColor;
	lValue.bkgndColor = bkgndColor;
	children.push_back(&lName);
	children.push_back(&lValue);
	children.push_back(&graph);
}

void BaseChangeComponent::drawChunk(area_t chunk, uint16_t *buf) {
	// first fill with the background/edge color as needed
	uint16_t *px = nullptr;
	for (uint16_t x = chunk.x; x < (chunk.x + chunk.w); x++) {
		for (uint16_t y = chunk.y; y < (chunk.y + chunk.h); y++) {
			px = getPixel(buf, chunk.w, chunk.h, x - chunk.x, y - chunk.y);
			// check if we're on the margin
			if ((x < margin) || (x >= (area.x + area.w) - margin)
					|| (y < margin) || (y >= (area.y + area.h))) {
				*px = (uint16_t) edgeColor;
			} else {
				*px = (uint16_t) bkgndColor;
			}

		}
	}
	// now draw any relevant child components
	for (auto *child : children) {
		if (hasOverlap(chunk, child->getArea()))
			child->drawChunk(chunk, buf);
	}

}

void BaseChangeComponent::placeChildren() {
	// little idea maybe
	const uint16_t dX = area.w / 12;
	const uint16_t dY = area.h / 9;

	area_t nameArea;
	nameArea.x = dX;
	nameArea.y = dY;
	nameArea.w = dX * 10;
	nameArea.h = lName.getIdealHeight(2);
	lName.setArea(nameArea);

	area_t valueArea;
	valueArea.x = dX;
	valueArea.y = 2 * dY;
	valueArea.w = dX * 10;
	valueArea.h = lValue.getIdealHeight(2);
	lValue.setArea(valueArea);

	area_t graphArea;
	graphArea.x = 2 * dX;
	graphArea.y = 4 * dY;
	graphArea.w = dX * 8;
	graphArea.h = dY * 2;
	graph.setArea(graphArea);
	graph.setVertical(false);

}

void BaseChangeComponent::prepareToShow(const std::string &name,
		const std::string &value, int16_t level, int16_t maxLevel) {
	lName.setText(name);
	lValue.setText(value);
	graph.setMaxLevel(maxLevel);
	graph.setLevel(level);
	//NOTE: we don't always want the bar graph and we can indicate that by passing a maxLevel < 0
	if (maxLevel < 0)
		forbiddenComp = &graph;
	else
		forbiddenComp = nullptr;
}

// mod comp----------------------------------------
ModChangeComponent::ModChangeComponent() {
	// set up the header component
	lHeader.setFont(&Font_11x18);
	lHeader.setText("Selected Modulation:");
	graph.setMaxLevel(128);
	graph.setVertical(false);
	children.push_back(&lHeader);
	children.push_back(&lName);
	children.push_back(&lValue);
	children.push_back(&graph);
}

void ModChangeComponent::placeChildren() {

	const uint16_t dX = area.w / 12;
	const uint16_t dY = area.h / 9;

	area_t headerArea;
	headerArea.x = 2 * dX;
	headerArea.y = dY;
	headerArea.w = dX * 10;
	headerArea.h = lHeader.getIdealHeight(2);
	lHeader.setArea(headerArea);

	area_t nameArea;
	nameArea.x = dX;
	nameArea.y = 3 * dY;
	nameArea.w = dX * 10;
	nameArea.h = lName.getIdealHeight(2);
	lName.setArea(nameArea);

	area_t valueArea;
	valueArea.x = dX;
	valueArea.y = 4 * dY;
	valueArea.w = dX * 10;
	valueArea.h = lValue.getIdealHeight(2);
	lValue.setArea(valueArea);

	area_t graphArea;
	graphArea.x = 2 * dX;
	graphArea.y = 6 * dY;
	graphArea.w = dX * 8;
	graphArea.h = dY * 2;
	graph.setArea(graphArea);
}

// helpers for setting the label text
std::string modSourceString(uint8_t id) {
	switch (id) {
	case ModSource::ENV1:
		return "ADSR 1";
	case ModSource::ENV2:
		return "ADSR 2";
	case ModSource::LFO1:
		return "LFO 1";
	case ModSource::LFO2:
		return "LFO 2";
	case ModSource::LFO3:
		return "LFO 3";
	case ModSource::MODWHL:
		return "Mod Wheel";
	case ModSource::PITCHWHL:
		return "Pitch Wheel";
	case ModSource::VEL:
		return "Note Velocity";
	default:
		return "NONE";
	}
	return "";
}

std::string modDestString(uint8_t id) {
	switch (id) {
	case ModDest::CUTOFF:
		return "Filter Cutoff";
	case ModDest::RESONANCE:
		return "Filter Resonance";
	case ModDest::FOLD:
		return "Wavefolder Amount";
	case ModDest::PWM1:
		return "Osc 1 PWM";
	case ModDest::PWM2:
		return "Osc 2 PWM";
	case ModDest::TUNE1:
		return "Osc 1 Detune";
	case ModDest::TUNE2:
		return "Osc 2 Detune";
	case ModDest::VCA:
		return "VCA";
	default:
		return "null";
	}

}

void ModChangeComponent::prepareToShow(uint8_t source, uint8_t dest,
		int8_t depth) {
	std::string val = modSourceString(source) + " >> " + modDestString(dest);
	lName.setText(val);
	lValue.setText(std::to_string(depth));
	graph.setLevel(depth);

}

void ModChangeComponent::drawChunk(area_t chunk, uint16_t *buf) {

	uint16_t *px = nullptr;
	for (uint16_t x = chunk.x; x < (chunk.x + chunk.w); x++) {
		for (uint16_t y = chunk.y; y < (chunk.y + chunk.h); y++) {
			px = getPixel(buf, chunk.w, chunk.h, x - chunk.x, y - chunk.y);
			// check if we're on the margin
			if ((x < margin) || (x >= (area.x + area.w) - margin)
					|| (y < margin) || (y >= (area.y + area.h))) {
				*px = (uint16_t) edgeColor;
			} else {
				*px = (uint16_t) bkgndColor;
			}

		}
	}
	// now draw any relevant child components
	for (auto *child : children) {
		if (hasOverlap(chunk, child->getArea()))
			child->drawChunk(chunk, buf);
	}
}

// view -------------------
ModalChangeView::ModalChangeView() {

}

void ModalChangeView::initChildren() {
	constexpr uint16_t dX = ILI9341_WIDTH / 8;
	constexpr uint16_t dY = ILI9341_HEIGHT / 8;
	area_t modalArea;
	modalArea.x = dX;
	modalArea.y = dY;
	modalArea.w = 6 * dX;
	modalArea.h = 6 * dY;

	baseModal.setArea(modalArea);
	baseModal.placeChildren();
	modModal.setArea(modalArea);
	modModal.placeChildren();

	children.push_back(&baseModal);
	//children.push_back(&modModal);
}

void ModalChangeView::paramUpdated(uint8_t id) {
	lastUpdateTick = TickTimer_get();
	/** COMPLETE LIST OF PARAMS THIS NEEDS TO HANDE (as of 9/7):
	 * - Pulse width 1: value/4096
	 * - Pulse width 2
	 * - Filter cutoff
	 * - Filter resonance
	 * - Filter mode (hi/low pass)
	 * - Fold level
	 * - Fold Order (whether filter -> folder or folder -> filter)
	 */
	std::string nameStr, valStr;
	switch (id) {
	case ParamID::pOsc1PulseWidth:
		nameStr = "DCO 1 Pulse Width";
		valStr = std::to_string(patch->oscs[0].pulseWidth) + "/4096";
		baseModal.prepareToShow(nameStr, valStr, patch->oscs[0].pulseWidth,
				4096);
		break;
	case ParamID::pOsc2PulseWidth:
		nameStr = "DCO 2 Pulse Width";
		valStr = std::to_string(patch->oscs[1].pulseWidth) + "/4096";
		baseModal.prepareToShow(nameStr, valStr, patch->oscs[1].pulseWidth,
				4096);
		break;
	case ParamID::pFilterCutoff:
		nameStr = "Filter Cutoff";
		valStr = std::to_string(patch->cutoffBase) + "/4096";
		baseModal.prepareToShow(nameStr, valStr, patch->cutoffBase, 4096);
		break;
	case ParamID::pFilterRes:
		nameStr = "Filter Resonance";
		valStr = std::to_string(patch->resBase) + "/256";
		baseModal.prepareToShow(nameStr, valStr, patch->resBase, 256);
		break;
	case ParamID::pFilterMode:
		nameStr = "Filter Mode";
		valStr = (patch->highPassMode > 0) ? "High Pass" : "Low Pass";
		// the -1 indicates that we won't show the graph for this
		baseModal.prepareToShow(nameStr, valStr, 0, -1);
		break;
	case ParamID::pFoldLevel:
		nameStr = "Wavefolder Level";
		valStr = std::to_string(patch->foldBase) + "/4096";
		baseModal.prepareToShow(nameStr, valStr, patch->foldBase, 4096);
		break;
	case ParamID::pFoldFirst:
		nameStr = "Wavefolder/Filter Order";
		valStr =
				(patch->foldFirst > 0) ?
						"Wavefolder > Filter" : "Filter > Wavefolder";
		baseModal.prepareToShow(nameStr, valStr, 0, -1);
		break;
	default:
		break;
	}
	baseModal.draw();
}

bool ModalChangeView::timeToRemove() {
	constexpr float modalLengthMs = 3100.0f;
	return TickTimer_tickDistanceMs(lastUpdateTick, TickTimer_get())
			>= modalLengthMs;
}




