/*
 * Component.h
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */

#ifndef INC_GRAPHICS_COMPONENT_H_
#define INC_GRAPHICS_COMPONENT_H_
#include <Color.h>
#include "main.h"
#include "ili9341.h"
#include <stdbool.h>
#include "Patch.h"
#include "DisplayQueue.h"
#define MAX_CHUNK_WIDTH 64
#define MAX_CHUNK_HEIGHT 64
#define MAX_CHUNK_PX 4096
#define MAX_CHUNK_BYTES 8192

// some C stuff-------------
#ifdef __cplusplus
extern "C" {
#endif
area_t getOverlap(area_t a, area_t b);
bool hasOverlap(area_t a, area_t b);
bool pointInArea(area_t a, uint16_t x, uint16_t y);

uint16_t numChunksNeeded(area_t area);

// line stuff
typedef struct {
	point_t a;
	point_t b;
} line_t;

// pixel access
uint16_t* getPixel(uint16_t *buf, uint16_t width, uint16_t height, uint16_t x,
		uint16_t y);
uint16_t* pixelInChunk(uint16_t buf, area_t area);

void fillChunk(uint16_t *buf, uint16_t w, uint16_t h, color16_t color);

#ifdef __cplusplus
}
#endif
//--------------------------------------------------

#ifdef __cplusplus
#include <functional>
#include <memory>
#include <string>
#include <vector>




// COMPONENT  ==================================
class Component {
protected:
	DisplayQueue* const queue;
	area_t area;
	uint8_t zIndex;
	// this splits the component up into task-sized chunks
	std::vector<area_t> getTaskChunks();
public:
	Component();
	virtual ~Component();
	area_t getArea(){
		return area;
	}
	void setArea(area_t a);
	void setZIndex(uint8_t val);
	// this needs to be overridden by all subclasses
	virtual void drawChunk(area_t chunk, uint16_t *buf)=0;
	// this can be called from the processor to draw all the chunks of this component
	void draw();

protected:
	DrawTask drawTaskForChunk(area_t chunk);
	// helpful later
	friend class View;
};

// VIEW==============================================

class View {
protected:
	std::vector<Component*> children;
	std::string viewName = "Null View";
public:
	View();
	virtual ~View();
	void setName(const std::string& str);
	// override this to set the child components' positions and add the pointers to our children vector
	virtual void initChildren()=0;
	virtual void paramUpdated(uint8_t id){
	}
	void draw();
	void redrawChunks(std::vector<area_t>& chunks);
};

#endif

#ifdef __cplusplus
#define EXTERNC extern "c"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_GRAPHICS_COMPONENT_H_ */
