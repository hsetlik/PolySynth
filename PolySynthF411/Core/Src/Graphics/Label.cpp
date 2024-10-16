/*
 * Label.cpp
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */

#include "Label.h"


Label::Label(const std::string &s) :
		text(s), font(&Font_7x10) {

}

Label::Label() :
		text("Label"), font(&Font_7x10) {

}

void Label::setFont(FontDef *f) {
	font = f;
}

void Label::setText(const std::string &str) {
	if (str != text) {
		text = str;
		needsRedraw = true;
	}
}

uint16_t Label::getIdealWidth(uint16_t margin) {
	return (2 * margin) + (uint16_t) (text.length() * font->width);
}

uint16_t Label::getIdealHeight(uint16_t margin) {
	return (2 * margin) + (uint16_t) font->height;
}

// helpers for drawing strings
area_t charArea(FontDef *font, uint8_t idx, uint16_t x0, uint16_t y0) {
	area_t a;
	a.x = x0 + (font->width * idx);
	a.y = y0;
	a.w = font->width;
	a.h = font->height;
	return a;
}

void drawStringChunk(FontDef *font, uint16_t *buf, area_t bufArea,
		const std::string &str, area_t strArea, color16_t textColor) {
	area_t currentCharArea { 0, 0, 0, 0 };
	uint8_t idx = 0;

	// find the first chracter that needs to be visible with this chunk
	while (!hasOverlap(currentCharArea, bufArea)) {
		currentCharArea = charArea(font, idx, strArea.x, strArea.y);
		++idx;
	}
	uint16_t xBuf = (strArea.x > bufArea.x) ? strArea.x - bufArea.x : 0;
	const uint16_t yBuf = (strArea.y > bufArea.y) ? strArea.y - bufArea.y : 0;
	uint16_t f;

	while (hasOverlap(currentCharArea, bufArea)) {
		// draw this character unless it's a space
		if (str[idx] != ' ') {
			for (uint16_t yChar = 0; yChar < font->height; yChar++) {
				char c = str[idx];
				f = font->data[(c - 32) * font->height + yChar];
				for (uint16_t xChar = 0; xChar < font->width; xChar++) {
					if (pointInArea(bufArea, xBuf + xChar, yBuf + yChar)) {
						uint16_t *px = getPixel(buf, bufArea.w, bufArea.h, xBuf,
								yBuf);
						if ((f << xChar) & 0x8000) {
							*px = textColor;
						}
					}
				}

			}
		}
		// move on to the next character
		idx++;
		currentCharArea = charArea(font, idx, strArea.x, strArea.y);
		xBuf += font->width;
	}
}

void Label::drawChunk(area_t chunk, uint16_t *buf) {
	if (needsRedraw)
		needsRedraw = false;
	if (!hasOverlap(area, chunk))
		return;
	fillChunk(buf, chunk.w, chunk.h, bkgndColor);
	drawStringChunk(font, buf, chunk, text, getStringArea(), txtColor);
}

area_t Label::getStringArea() {
	area_t a;
	a.x = area.x + 3;
	a.y = area.y + 2;
	a.w = font->width * text.length();
	a.h = (uint16_t) font->height;
	return a;
}






