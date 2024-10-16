/*
 * Component.cpp
 *
 *  Created on: Oct 16, 2024
 *      Author: hayden
 */
#include "Component.h"

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

bool hasOverlap(area_t a, area_t b) {
	uint16_t aRight = a.x + a.w;
	uint16_t aBottom = a.y + a.h;
	uint16_t bRight = b.x + b.w;
	uint16_t bBottom = b.y + b.h;
	bool xOverlap = ((aRight > b.x) && (a.x <= bRight))
			| ((bRight > a.x) && (b.x <= aRight));
	bool yOverlap = ((aBottom > b.y) && (a.y <= bBottom))
			| ((bBottom > a.y) && (b.y <= aBottom));
	return xOverlap && yOverlap;
}

bool pointInArea(area_t a, uint16_t x, uint16_t y) {
	return (x < (a.x + a.w) && x >= a.x) && (y < (a.y + a.h) && y >= a.y);
}

uint16_t numChunksNeeded(area_t area) {
	uint16_t columns =
			(area.w <= MAX_CHUNK_WIDTH) ? 1 : ((area.w / MAX_CHUNK_WIDTH) + 1);
	uint16_t rows =
			(area.h <= MAX_CHUNK_HEIGHT) ?
					1 : ((area.h / MAX_CHUNK_HEIGHT) + 1);
	return rows * columns;
}

uint16_t* getPixel(uint16_t *buf, uint16_t width, uint16_t height, uint16_t x,
		uint16_t y) {
	uint16_t row = y / height;
	return &buf[(row * width) + x];
}

void fillChunk(uint16_t *buf, uint16_t w, uint16_t h, color16_t color) {
	// I think O(n) complexity is the best we can do here :(
	for (uint16_t i = 0; i < (w * h); i++)
		buf[i] = color;
}

//======================================================

Component::Component() :
		queue(static_cast<DisplayQueue*>(mainDispQueue)) {

}

Component::~Component() {

}

void Component::setArea(area_t a) {
	area = a;
}

void Component::setZIndex(uint8_t val) {
	zIndex = val;
}

std::vector<area_t> Component::getTaskChunks() {
	std::vector<area_t> chunks;
	uint16_t currentX = area.x;
	uint16_t currentY = area.y;
	const uint16_t xLimit = area.x + area.w;
	const uint16_t yLimit = area.y + area.h;
	while (currentX < xLimit || currentY < yLimit) {
		area_t current;
		current.x = currentX;
		current.y = currentY;
		if (currentX >= xLimit) { // we've hit the right edge and now move down a row
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

void Component::draw() {
	std::vector<area_t> chunks = getTaskChunks();
	for (uint8_t c = 0; c < chunks.size(); c++) {
		DrawTask task = drawTaskForChunk(chunks[c]);
		queue->push(task);
	}
}

DrawTask Component::drawTaskForChunk(area_t chunk) {
	DrawTask task;
	task.area = chunk;
	task.func = [this](area_t a, uint16_t *buf) {
		this->drawChunk(a, buf);
	};
	return task;
}


//VIEW==================================================
View::View() {

}

View::~View() {

}

void View::draw() {
	for (uint8_t i = 0; i < children.size(); i++) {
		children[i]->draw();
	}
}

void View::redrawChunks(std::vector<area_t> &chunks) {
	DisplayQueue *queue = static_cast<DisplayQueue*>(mainDispQueue);
	for (auto &chunk : chunks) {
		// check each child to see if it needs to draw this chunk
		// this is O(n * numComponents) but it should ultimately
		// save time by drawing fewer pixels in smaller tasks
		for (auto *child : children) {
			if (hasOverlap(chunk, child->area)) { // we have to redraw
				auto task = child->drawTaskForChunk(chunk);
				queue->push(task);
			}
		}
	}
}

void View::setName(const std::string &str) {
	viewName = str;
}

//======================================================

//---------------------------

