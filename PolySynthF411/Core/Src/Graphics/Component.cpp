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

// line stuff----------------------------
uint8_t tripletOrientation(point_t a, point_t b, point_t c) {

	int16_t val = (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.x - b.x);
	if (val == 0)
		return 0;
	return (val > 0) ? 1 : 2;
}

bool isOnLine(line_t line, point_t p) {
	if (p.x <= std::max(line.a.x, line.b.x)
			&& p.x >= std::min(line.a.x, line.b.x)
			&& p.y <= std::max(line.a.y, line.b.y)
			&& p.y >= std::min(line.a.y, line.b.y))
		return true;
	return false;
}

bool linesIntersect(line_t l1, line_t l2) {
	uint8_t o1 = tripletOrientation(l1.a, l1.b, l2.a);
	uint8_t o2 = tripletOrientation(l1.a, l1.b, l2.b);
	uint8_t o3 = tripletOrientation(l2.a, l2.b, l1.a);
	uint8_t o4 = tripletOrientation(l2.a, l2.b, l1.b);

	if (o1 != o2 && o3 != o4)
		return true;
	if (o1 == 0 && isOnLine(l1, l2.a))
		return true;
	if (o2 == 0 && isOnLine(l1, l2.b))
		return true;
	if (o3 == 0 && isOnLine(l2, l1.a))
		return true;
	if (o4 == 0 && isOnLine(l2, l1.b))
		return true;

	return false;
}

bool isLineInChunk(line_t line, area_t area) {
	// create the four sides of the chunk as lines, return true if any intersect
	line_t left;
	left.a.x = (int16_t) area.x;
	left.a.y = (int16_t) area.y;
	left.b.x = (int16_t) area.x;
	left.b.y = (int16_t) area.y + area.h;
	if (linesIntersect(line, left))
		return true;

	line_t top;
	top.a.x = (int16_t) area.x;
	top.a.y = (int16_t) area.y;
	top.b.x = (int16_t) area.x + area.w;
	top.b.y = (int16_t) area.y;
	if (linesIntersect(line, top))
		return true;

	line_t right;
	right.a.x = (int16_t) area.x + area.w;
	right.a.y = (int16_t) area.y;
	right.b.x = (int16_t) area.x + area.w;
	right.b.y = (int16_t) area.y + area.h;
	if (linesIntersect(line, right))
		return true;

	line_t bottom;
	bottom.a.x = (int16_t) area.x;
	bottom.a.y = (int16_t) area.y + area.h;
	bottom.b.x = (int16_t) area.x + area.w;
	bottom.b.y = (int16_t) area.y + area.h;
	if (linesIntersect(line, bottom))
		return true;

	return false;
}

void drawLineInChunk(line_t line, area_t chunk, uint16_t *buf,
		color16_t lineCol) {
	// it's easier to work with the points in terms of left and right
	point_t *left = (line.a.x < line.b.x) ? &line.a : &line.b;
	point_t *right = (left == &line.a) ? &line.b : &line.a;

	// this is Bresenham's line algorithm as explained here: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	if (std::abs(right->y - left->y) < std::abs(right->x - left->x)) {
		// equivalent to 'plotLineLow' in the wiki article
		int16_t dx = right->x - left->x;
		int16_t dy = right->y - left->y;
		int16_t yInc = 1;
		if (dy < 0) {
			yInc = -1;
			dy *= -1;
		}
		int16_t D = (2 * dy) - dx;
		int16_t currentY = left->y;
		for (int16_t x = left->x; x < right->x; x++) {
			if (pointInArea(chunk, (uint16_t) x, (uint16_t) currentY)) {
				uint16_t *px = getPixel(buf, chunk.w, chunk.h,
						(uint16_t) x - chunk.x, (uint16_t) currentY - chunk.y);
				*px = lineCol;

			}
			if (D > 0) {
				currentY += yInc;
				D = D + (2 * (dy - dx));
			} else {
				D = D + (2 * dy);
			}
		}

	} else {
		// equivalent to 'plotLineHigh' in the wiki article
		int16_t dx = right->x - left->x;
		int16_t dy = right->y - left->y;
		int16_t xInc = 1;

		if (dx > 0) {
			xInc = -1;
			dx *= -1;
		}
		int16_t D = (2 * dx) - dy;
		int16_t currentX = left->x;

		for (int16_t y = left->y; y < right->y; y++) {
			if (pointInArea(chunk, (uint16_t) currentX, (uint16_t) y)) {
				uint16_t *px = getPixel(buf, chunk.w, chunk.h,
						(uint16_t) currentX - chunk.x, (uint16_t) y - chunk.y);
				*px = lineCol;
			}
			if (D > 0) {
				currentX += xInc;
				D = D + (2 * (dx - dy));
			} else {
				D = D + (2 * dx);
			}
		}
	}
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

