/*
 * OscView.h
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */

#ifndef INC_GRAPHICS_OSCVIEW_H_
#define INC_GRAPHICS_OSCVIEW_H_
#include "Component.h"

#ifdef __cplusplus
#include "Label.h"
#include "BarGraph.h"

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



#endif

#ifdef __cplusplus
#define EXTERNC extern "c"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_GRAPHICS_OSCVIEW_H_ */
