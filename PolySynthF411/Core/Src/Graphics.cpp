/*
 * Graphics.cpp
 *
 *  Created on: Aug 22, 2024
 *      Author: hayden
 */

#include "Graphics.h"

area_t getOverlap(area_t a, area_t b) {

	area_t out;
	uint16_t aRight = a.x + a.w;
	uint16_t aBottom = a.y + a.h;
	uint16_t bRight = b.x + b.w;
	uint16_t bBottom = b.y + b.h;

	out.x = (a.x > b.x) ? a.x : b.x;
	uint16_t right = (aRight < bRight) ? aRight : bRight;
	out.w = right - out.x;
	out.y = (a.y > b.y) ? a.y : b.y;
	uint16_t bottom = (aBottom < bBottom) ? aBottom : bBottom;
	out.h = bottom - out.y;

	return out;
}

uint16_t numChunksNeeded(area_t area) {
	uint16_t columns =
			(area.w <= MAX_CHUNK_WIDTH) ? 1 : ((area.w / MAX_CHUNK_WIDTH) + 1);
	uint16_t rows =
			(area.h <= MAX_CHUNK_HEIGHT) ?
					1 : ((area.h / MAX_CHUNK_HEIGHT) + 1);
	return rows * columns;
}

//======================================================

Component::Component() {

}

Component::~Component() {

}

void Component::setArea(area_t a) {
	area = a;
}

void Component::setZIndex(uint8_t val){
	zIndex = val;
}


std::vector<area_t> Component::getTaskChunks(){
	std::vector<area_t> chunks;
	uint16_t currentX = area.x;
	uint16_t currentY = area.y;
	const uint16_t xLimit = area.x + area.w;
	const uint16_t yLimit = area.y + area.h;
	while(currentX < xLimit || currentY < yLimit){
		area_t current;
		current.x = currentX;
		current.y = currentY;
		if(currentX >= xLimit){ // we've hit the right edge and now move down a row
			current.w = xLimit - currentX;
			current.h = std::min<uint16_t>(area.h, MAX_CHUNK_HEIGHT);
			currentX = area.x;
			currentY += MAX_CHUNK_HEIGHT;
		} else { // otherwise move right
			current.w = std::min<uint16_t>(MAX_CHUNK_WIDTH, xLimit - currentX);
			current.h = std::min<uint16_t>(area.h, MAX_CHUNK_HEIGHT);
			currentX += MAX_CHUNK_WIDTH;
		}
		chunks.push_back(current);
	}
	return chunks;
}


void Component::draw(RingBuffer<DrawTask>& queue){
	std::vector<area_t> chunks = getTaskChunks();
	for(uint8_t c = 0; c < chunks.size(); c++){
		DrawTask task;
		task.area = chunks[c];
		task.func = [this](area_t area, uint16_t* buf){
			this->drawChunk(area, buf);
		};
		queue.push(task);
	}
}
//---------------------------
Label::Label(const std::string& s) : text(s), font(&Font_7x10){

}


Label::Label() : text("Label"), font(Font_7x10){

}

void Label::setFont(FontDef* f){
	font = f;
}


uint16_t Label::getIdealWidth(uint16_t margin){
	return (2 * margin) + (uint16_t)(text.length() * font->width);
}

uint16_t Label::getIdealHeight(uint16_t margin){
	return (2 * margin) + (uint16_t)font->height;
}


void Label::drawChunk(area_t chunk, uint16_t* buf){

}

//======================================================

GraphicsProcessor::GraphicsProcessor() :
		dmaBusy(false) {
	// initialize the screen
	ILI9341_Init();

}

void GraphicsProcessor::dmaFinished() {
	dmaBusy = false;
}

void GraphicsProcessor::checkDrawQueue() {
	if (!dmaBusy && !queue.empty()) {
		runFront();
	}
}

void GraphicsProcessor::pushTask(DrawTask task) {
	queue.push(task);
}

void GraphicsProcessor::runFront() {
	DrawTask t = queue.getFront();
	// run the callback to render the pixels
	t.func(t.area, dmaBuf);
	ILI9341_DrawImage_DMA(t.area.x, t.area.y, t.area.w, t.area.h, dmaBuf);
	dmaBusy = true;

}
//======================================================

graphics_processor_t create_graphics_processor() {
	return new GraphicsProcessor();
}

void disp_dma_finished(graphics_processor_t proc) {
	GraphicsProcessor *ptr = static_cast<GraphicsProcessor*>(proc);
	ptr->dmaFinished();
}

void check_draw_queue(graphics_processor_t proc) {
	GraphicsProcessor *ptr = static_cast<GraphicsProcessor*>(proc);
	ptr->checkDrawQueue();
}

