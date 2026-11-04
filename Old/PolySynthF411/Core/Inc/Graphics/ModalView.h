/*
 * ModalView.h
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */

#ifndef INC_GRAPHICS_MODALVIEW_H_
#define INC_GRAPHICS_MODALVIEW_H_
#include "Component.h"


#ifdef __cplusplus
#include "Label.h"
#include "BarGraph.h"


// this is for changes to BASE parameters
class BaseChangeComponent : public Component {
private:
// components
	Label lName;
	Label lValue;
	BarGraph graph;
// drawing state
	uint16_t margin = 7;
	color16_t bkgndColor = color565_getColor16(ColorID::Salmon);
	color16_t edgeColor = color565_getColor16(ColorID::Maroon);
	// this should palce all child components relative to this component's area
	void placeChildren();
	std::vector<Component*> children;
	Component* forbiddenComp = nullptr;
	// this gets called from ModalChangeView's paramUpdated method
	void prepareToShow(const std::string& name, const std::string& value, int16_t level, int16_t maxLevel);

public:
	BaseChangeComponent();
	void drawChunk(area_t chunk, uint16_t *buf) override;
	friend class ModalChangeView;
};

// this is similar to the above but for changes to modulations
class ModChangeComponent : public Component {
private:
	Label lHeader;
	Label lName;
	Label lValue;
	BipolarBarGraph graph;

	uint16_t margin = 7;
	color16_t bkgndColor = color565_getColor16(ColorID::Salmon);
	color16_t edgeColor = color565_getColor16(ColorID::Maroon);

	void placeChildren();
	std::vector<Component*> children;
	// this sets up the labels and graphs
	void prepareToShow(uint8_t source, uint8_t dest, int8_t depth);
public:
	ModChangeComponent();
	void drawChunk(area_t chunk, uint16_t *buf) override;
	friend class ModalChangeView;

};

// view------

class ModalChangeView : public View {
private:
	patch_t* patch;
	tick_t lastUpdateTick = 0;

	BaseChangeComponent baseModal;
	ModChangeComponent modModal;
public:
	ModalChangeView();
	void initChildren() override;
	//Note: in this case updating the param causes this view to become visible
	void paramUpdated(uint8_t id) override;
	void setPatch(patch_t* p) {
		patch = p;
	}
	// check if this component has been visible for longer that the alloted time
	bool timeToRemove();

	//get the area we need to redraw
	std::vector<area_t> getChunksForArea(){
		return baseModal.getTaskChunks();
	}

};



#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_GRAPHICS_MODALVIEW_H_ */
