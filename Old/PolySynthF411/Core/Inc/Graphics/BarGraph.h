/*
 * BarGraph.h
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */

#ifndef INC_GRAPHICS_BARGRAPH_H_
#define INC_GRAPHICS_BARGRAPH_H_
#include "Component.h"

#ifdef __cplusplus

class BarGraph : public Component {
private:
	uint16_t maxLevel;
	uint16_t currentLevel = 0;
	uint16_t margin = 3;
	bool isVertical = true;
	area_t getBarArea();
public:
	color16_t barColor = color565_getColor16(ColorID::Maroon);
	color16_t bkgndColor = color565_getColor16(ColorID::Salmon);
	BarGraph(uint16_t maxValue=255);
	void setLevel(uint16_t lvl){
		currentLevel = lvl;
	}
	void drawChunk(area_t chunk, uint16_t *buf) override;
	void setMaxLevel(int16_t val) {
		maxLevel = val;
	}
	void setVertical(bool v){
		isVertical = v;
	}
};

class BipolarBarGraph : public Component {
private:
	int16_t maxLevel;
	int16_t currentLevel = 0;
	uint16_t margin = 3;
	bool isVertical = true;
	area_t getBarArea();
public:
	color16_t negBarColor = color565_getColor16(ColorID::Maroon);
	color16_t posBarColor = color565_getColor16(ColorID::DarkGreen);
	color16_t bkgndColor = color565_getColor16(ColorID::Salmon);
	BipolarBarGraph(int16_t max=127);
	void drawChunk(area_t chunk, uint16_t *buf) override;
	void setLevel(int16_t lvl) {
		currentLevel = lvl;
	}
	void setVertical(bool v){
		isVertical = v;
	}
	void setMaxLevel(int16_t val) {
		maxLevel = val;
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

#endif /* INC_GRAPHICS_BARGRAPH_H_ */
