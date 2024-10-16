/*
 * BarGraph.cpp
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */
#include "BarGraph.h"

BarGraph::BarGraph(uint16_t maxVal) :
		maxLevel(maxVal) {

}

area_t BarGraph::getBarArea() {
	area_t a;
	if (isVertical) {
		float fLevel = (float) currentLevel / (float) maxLevel;
		a.h = (uint16_t) ((1.0f - fLevel) * (float) (area.h - (2 * margin)));
		a.x = area.x + margin;
		a.y = (area.y + area.h) - (a.h + margin);
		a.w = area.w - (2 * margin);
	} else {
		a.x = area.x + margin;
		float fLevel = (float) currentLevel / (float) maxLevel;
		a.w = (uint16_t) ((1.0f - fLevel) * (float) (area.w - (2 * margin)));
		a.y = area.y + margin;
		a.h = area.h - (2 * margin);
	}
	return a;
}

void BarGraph::drawChunk(area_t chunk, uint16_t *buf) {
	if (!hasOverlap(chunk, area))
		return;
	area_t barArea = getBarArea();
	for (uint16_t x = chunk.x; x < chunk.x + chunk.w; x++) {
		for (uint16_t y = chunk.y; y < chunk.y + chunk.h; y++) {
			uint16_t *px = getPixel(buf, chunk.w, chunk.h, x - chunk.x,
					y - chunk.y);
			if (pointInArea(barArea, x, y)) {
				*px = barColor;
			} else {
				*px = bkgndColor;
			}
		}
	}
}

BipolarBarGraph::BipolarBarGraph(int16_t max) :
		maxLevel(max) {

}

area_t BipolarBarGraph::getBarArea() {
	area_t bar;
	if (isVertical) {
		bar.x = area.x + margin; // the x and width will always be the same
		bar.w = area.w - (2 * margin);
		// now check if we're positive or negative
		const uint16_t centerY = area.y + (area.h / 2);
		const uint16_t maxBarHeight = (area.h / 2) - margin;
		float fHeight = (float) std::abs(currentLevel) / (float) maxLevel;
		uint16_t barHeight = (uint16_t) (fHeight * (float) maxBarHeight);
		if (currentLevel >= 0) { // positive value
			bar.y = centerY - barHeight;
			bar.h = barHeight;
		} else { // negative value
			bar.y = centerY;
			bar.h = barHeight;
		}
	} else { // horizontal;
		bar.y = area.y + margin;
		bar.h = area.h - (2 * margin);
		const uint16_t centerX = area.x + (area.w / 2);
		const uint16_t maxBarWidth = (area.w / 2) - margin;
		float fWidth = (float) std::abs(currentLevel) / (float) maxLevel;
		const uint16_t barWidth = (uint16_t) (fWidth * (float) maxBarWidth);
		if (currentLevel >= 0) { // positive-> bar on the right
			bar.x = centerX;
			bar.w = barWidth;
		} else { // negative -> bar on the left
			bar.x = centerX - barWidth;
			bar.w = barWidth;
		}
	}
	return bar;
}

void BipolarBarGraph::drawChunk(area_t chunk, uint16_t *buf) {

	if (!hasOverlap(chunk, area))
		return;
	color16_t *barColor = (currentLevel >= 0) ? &posBarColor : &negBarColor;
	area_t barArea = getBarArea();
	for (uint16_t x = chunk.x; x < chunk.x + chunk.w; x++) {
		for (uint16_t y = chunk.y; y < chunk.y + chunk.h; y++) {
			uint16_t *px = getPixel(buf, chunk.w, chunk.h, x - chunk.x,
					y - chunk.y);
			if (pointInArea(barArea, x, y)) {
				*px = *barColor;
			} else {
				*px = bkgndColor;
			}
		}
	}
}








