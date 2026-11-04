/*
 * EnvView.h
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */

#ifndef INC_GRAPHICS_ENVVIEW_H_
#define INC_GRAPHICS_ENVVIEW_H_
#include "Component.h"

#ifdef __cplusplus
#include "Label.h"
#include "BarGraph.h"
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



#endif

#ifdef __cplusplus
#define EXTERNC extern "c"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_GRAPHICS_ENVVIEW_H_ */
