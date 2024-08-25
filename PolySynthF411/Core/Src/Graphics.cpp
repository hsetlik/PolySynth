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

GraphicsProcessor::GraphicsProcessor() :
		dmaBusy(false) {
	// initialize the screen
	ILI9341_Init();

}

void GraphicsProcessor::dmaFinished() {
	// our DMA transmission from runFront() is finished so we'll increment the front now
	dmaBusy = false;
}


void GraphicsProcessor::checkDrawQueue(){
	if(!dmaBusy && !queue.empty()){
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

void check_draw_queue(graphics_processor_t proc){
	GraphicsProcessor *ptr = static_cast<GraphicsProcessor*>(proc);
	ptr->checkDrawQueue();
}


