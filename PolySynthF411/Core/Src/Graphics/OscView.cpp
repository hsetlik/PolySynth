/*
 * OscView.cpp
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */
#include "OscView.h"
// TUNING VIEW=============================================
OscTuningView::OscTuningView() {

}

void OscTuningView::initChildren() {
	// set the name label
	static const uint16_t nameHeight = 24; // for 11x18 font
	nameLabel.setFont(&Font_11x18);
	nameLabel.setText(viewName);
	nameLabel.setArea( { 0, 0, ILI9341_WIDTH, nameHeight });

	static const uint16_t sectionW = ILI9341_WIDTH / 4;

	// set up each osc's name label
	lOsc1.setFont(&Font_11x18);
	lOsc1.setText("DCO 1");
	lOsc1.setArea( { 0, nameHeight, (uint16_t) (2 * sectionW), nameHeight });

	lOsc2.setFont(&Font_11x18);
	lOsc2.setText("DCO 2");
	lOsc2.setArea(
			{ (uint16_t) (2 * sectionW), nameHeight, (uint16_t) (2 * sectionW),
					nameHeight });

	// now the labels below each
	uint16_t currentX = 0;
	static const uint16_t labelHeight = 14;
	static const uint16_t valueY = ILI9341_HEIGHT - labelHeight;
	static const uint16_t nameY = valueY - labelHeight;
	static const uint16_t graphY = 2 * nameHeight;
	static const uint16_t graphHeight = nameY - graphY;

	// coarse tune 1
	gCoarse1.setArea( { currentX, graphY, sectionW, graphHeight });
	lCoarseName1.setArea( { currentX, nameY, sectionW, labelHeight });
	lCoarseVal1.setArea( { currentX, valueY, sectionW, labelHeight });
	gCoarse1.setLevel((int16_t) params1->coarseTune);
	lCoarseName1.setText("COARSE");
	lCoarseVal1.setText(std::to_string(params1->coarseTune));
	currentX += sectionW;

	// fine tune 1
	gFine1.setArea( { currentX, graphY, sectionW, graphHeight });
	lFineName1.setArea( { currentX, nameY, sectionW, labelHeight });
	lFineVal1.setArea( { currentX, valueY, sectionW, labelHeight });
	gFine1.setLevel((int16_t) params1->fineTune);
	lFineName1.setText("COARSE");
	lFineVal1.setText(std::to_string(params1->fineTune));
	currentX += sectionW;

	// coarse tune 2
	gCoarse2.setArea( { currentX, graphY, sectionW, graphHeight });
	lCoarseName2.setArea( { currentX, nameY, sectionW, labelHeight });
	lCoarseVal2.setArea( { currentX, valueY, sectionW, labelHeight });
	gCoarse2.setLevel((int16_t) params2->coarseTune);
	lCoarseName2.setText("COARSE");
	lCoarseVal2.setText(std::to_string(params2->coarseTune));
	currentX += sectionW;

	// fine tune 2
	gFine2.setArea( { currentX, graphY, sectionW, graphHeight });
	lFineName2.setArea( { currentX, nameY, sectionW, labelHeight });
	lFineVal2.setArea( { currentX, valueY, sectionW, labelHeight });
	gFine2.setLevel((int16_t) params2->fineTune);
	lFineName2.setText("COARSE");
	lFineVal2.setText(std::to_string(params2->fineTune));

	// now add pointers to the children vector
	children.push_back(&nameLabel);

	children.push_back(&lOsc1);
	children.push_back(&lFineName1);
	children.push_back(&lFineVal1);
	children.push_back(&gFine1);
	children.push_back(&lCoarseName1);
	children.push_back(&lCoarseVal1);
	children.push_back(&gCoarse1);

	children.push_back(&lOsc2);
	children.push_back(&lFineName2);
	children.push_back(&lFineVal2);
	children.push_back(&gFine2);
	children.push_back(&lCoarseName2);
	children.push_back(&lCoarseVal2);
	children.push_back(&gCoarse2);

}

void OscTuningView::paramUpdated(uint8_t id) {
	ParamID p = (ParamID) id;
	int16_t val = 0;
	switch (p) {
	case ParamID::pOsc1Coarse:
		val = (int16_t) params1->coarseTune;
		lCoarseVal1.setText(std::to_string(val));
		gCoarse1.setLevel(val);
		lCoarseVal1.draw();
		gCoarse1.draw();
		break;
	case ParamID::pOsc1Fine:
		val = (int16_t) params1->fineTune;
		lFineVal1.setText(std::to_string(val));
		gFine1.setLevel(val);
		lFineVal1.draw();
		gFine1.draw();
		break;
	case ParamID::pOsc2Coarse:
		val = (int16_t) params2->coarseTune;
		lCoarseVal2.setText(std::to_string(val));
		gCoarse2.setLevel(val);
		lCoarseVal2.draw();
		gCoarse2.draw();
		break;
	case ParamID::pOsc2Fine:
		val = (int16_t) params2->fineTune;
		lFineVal2.setText(std::to_string(val));
		gFine2.setLevel(val);
		lFineVal2.draw();
		gFine2.draw();
		break;
	default:
		break;
	}
}

//MIXER VIEW============================================

MixerView::MixerView() {

}

void MixerView::initChildren() {
	// set the name label
	static const uint16_t nameHeight = 24; // for 11x18 font
	nameLabel.setFont(&Font_11x18);
	nameLabel.setText(viewName);
	nameLabel.setArea( { 0, 0, ILI9341_WIDTH, nameHeight });

	static const uint16_t labelWidth = ILI9341_WIDTH / 4;
	static const uint16_t labelHeight = 14; // for 7x10 font
	static const uint16_t valueY = ILI9341_HEIGHT - labelHeight;
	static const uint16_t paramY = valueY - labelHeight;
	static const uint16_t graphHeight = ILI9341_HEIGHT
			- (nameHeight + (labelHeight * 2));
	uint16_t labelX = 0;
	// set up each name label and its corresponding param label
	sawNameLabel.setText("Sawtooth");
	sawNameLabel.setArea( { labelX, paramY, labelWidth, labelHeight });
	sawLevelLabel.setText(std::to_string(params->sawLevel));
	sawLevelLabel.setArea( { labelX, valueY, labelWidth, labelHeight });
	sawLevelGraph.setArea( { labelX, nameHeight, labelWidth, graphHeight });
	sawLevelGraph.setLevel((uint16_t) params->sawLevel);
	labelX += labelWidth;

	triNameLabel.setText("Triangle");
	triNameLabel.setArea( { labelX, paramY, labelWidth, labelHeight });
	triLevelLabel.setText(std::to_string(params->triLevel));
	triLevelLabel.setArea( { labelX, valueY, labelWidth, labelHeight });
	triLevelGraph.setArea( { labelX, nameHeight, labelWidth, graphHeight });
	triLevelGraph.setLevel((uint16_t) params->triLevel);
	labelX += labelWidth;

	pulseNameLabel.setText("Pulse");
	pulseNameLabel.setArea( { labelX, paramY, labelWidth, labelHeight });
	pulseLevelLabel.setText(std::to_string(params->pulseLevel));
	pulseLevelLabel.setArea( { labelX, valueY, labelWidth, labelHeight });
	pulseLevelGraph.setArea( { labelX, nameHeight, labelWidth, graphHeight });
	pulseLevelGraph.setLevel((uint16_t) params->pulseLevel);
	labelX += labelWidth;

	masterNameLabel.setText("Master");
	masterNameLabel.setArea( { labelX, paramY, labelWidth, labelHeight });
	masterLevelLabel.setText(std::to_string(params->oscLevel));
	masterLevelLabel.setArea( { labelX, valueY, labelWidth, labelHeight });
	masterLevelGraph.setArea( { labelX, nameHeight, labelWidth, graphHeight });
	masterLevelGraph.setLevel((uint16_t) params->oscLevel);
	labelX += labelWidth;

	children.push_back(&nameLabel);
	children.push_back(&sawNameLabel);
	children.push_back(&sawLevelLabel);
	children.push_back(&sawLevelGraph);
	children.push_back(&triNameLabel);
	children.push_back(&triLevelLabel);
	children.push_back(&triLevelGraph);
	children.push_back(&pulseNameLabel);
	children.push_back(&pulseLevelLabel);
	children.push_back(&pulseLevelGraph);
	children.push_back(&masterNameLabel);
	children.push_back(&masterLevelLabel);
	children.push_back(&masterLevelGraph);
}

void MixerView::paramUpdated(uint8_t id) {
	uint8_t squareIdx = (uint8_t) ParamID::pOsc1SquareLevel;
	if (id > (uint8_t) ParamID::pOsc1OscLevel
			&& id <= (uint8_t) ParamID::pOsc2OscLevel) {
		squareIdx = (uint8_t) ParamID::pOsc2SquareLevel;
	}
	//determine which param
	id -= squareIdx;
	switch (id) {
	case 0: // pulse
		pulseLevelLabel.setText(std::to_string(params->pulseLevel));
		pulseLevelLabel.draw();
		pulseLevelGraph.setLevel((uint16_t) params->pulseLevel);
		pulseLevelGraph.draw();
		break;
	case 1: // saw
		sawLevelLabel.setText(std::to_string(params->sawLevel));
		sawLevelLabel.draw();
		sawLevelGraph.setLevel((uint16_t) params->sawLevel);
		sawLevelGraph.draw();
		break;
	case 2: // tri
		triLevelLabel.setText(std::to_string(params->triLevel));
		triLevelLabel.draw();
		triLevelGraph.setLevel((uint16_t) params->triLevel);
		triLevelGraph.draw();
		break;
	case 3: // master
		masterLevelLabel.setText(std::to_string(params->oscLevel));
		masterLevelLabel.draw();
		masterLevelGraph.setLevel((uint16_t) params->oscLevel);
		masterLevelGraph.draw();
		break;
	default:
		break;
	}
}





