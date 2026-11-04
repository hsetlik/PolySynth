/*
 * Label.h
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */

#ifndef INC_GRAPHICS_LABEL_H_
#define INC_GRAPHICS_LABEL_H_

#include "Component.h"

#ifdef __cplusplus
class Label: public Component {
protected:
	std::string text;
	FontDef *font;
	bool needsRedraw = true;

	// drawing helper
	area_t getStringArea();
public:
	// colors
	color16_t txtColor = color565_getColor16(ColorID::White);
	color16_t bkgndColor = color565_getColor16(ColorID::Black);
	Label();
	Label(const std::string &s);
	void setFont(FontDef *f);
	void setText(const std::string& str);
	uint16_t getIdealWidth(uint16_t margin);
	uint16_t getIdealHeight(uint16_t margin);
	void drawChunk(area_t chunk, uint16_t *buf) override;
	bool textHasChanged() { return needsRedraw; }
};



#endif

#ifdef __cplusplus
#define EXTERNC extern "c"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_GRAPHICS_LABEL_H_ */
