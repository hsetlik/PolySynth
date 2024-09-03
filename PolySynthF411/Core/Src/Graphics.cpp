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

//======================================================

Component::Component() {

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

void Component::draw(RingBuffer<DrawTask> &queue) {
	std::vector<area_t> chunks = getTaskChunks();
	for (uint8_t c = 0; c < chunks.size(); c++) {
		DrawTask task;
		task.area = chunks[c];
		task.func = [this](area_t area, uint16_t *buf) {
			this->drawChunk(area, buf);
		};
		queue.push(task);
	}
}

//======================================================
View::View() {

}

View::~View() {

}

void View::draw(RingBuffer<DrawTask> &queue) {
	for (uint8_t i = 0; i < children.size(); i++) {
		children[i]->draw(queue);
	}
}

//======================================================
//---------------------------
Label::Label(const std::string &s) :
		text(s), font(&Font_7x10) {

}

Label::Label() :
		text("Label"), font(&Font_7x10) {

}

void Label::setFont(FontDef *f) {
	font = f;
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
		const std::string &str, area_t strArea, color16_t textColor,
		color16_t bkgndColor) {
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
		// draw this character
		for (uint16_t yChar = 0; yChar < font->height; yChar++) {
			char c = str[idx];
			f = font->data[(c - 32) * font->height + yChar];
			for (uint16_t xChar = 0; xChar < font->width; xChar++) {
				if (pointInArea(bufArea, xBuf + xChar, yBuf + yChar)) {
					uint16_t *px = getPixel(buf, bufArea.w, bufArea.h, xBuf,
							yBuf);
					if ((f << xChar) & 0x8000) {
						*px = textColor;
					} else {
						*px = bkgndColor;
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
	if (!hasOverlap(area, chunk))
		return;
	drawStringChunk(font, buf, chunk, text, getStringArea(), txtColor,
			bkgndColor);
}

area_t Label::getStringArea() {
	area_t a;
	a.x = area.x + 3;
	a.y = area.y + 2;
	a.w = font->width * text.length();
	a.h = (uint16_t) font->height;
	return a;
}

// ADSR graph===========================================

float totalLengthMs(adsr_t *env) {
	return env->attack + env->decay + env->release;
}

uint16_t flerp16(uint16_t a, uint16_t b, float t) {
	return a + (uint16_t) ((float) (b - a) * t);
}

// define how short/long the flat section representing sustain can be
// depending on the length of the envelope
#define SUSTAIN_WIDTH_MIN 0.16f
#define SUSTAIN_WIDTH_MAX 0.84f
/** This works like:
 * 	0- colinear
 * 	1- clockwise
 * 	2- counterclockwise
 *
 */

uint8_t tripletOrientation(point_t a, point_t b, point_t c) {

	int16_t val = (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.x - b.x);
	if (val == 0)
		return 0;
	return (val > 0) ? 1 : 2;
}

// line math stuff----------------
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

// get the points for a given envelope within a given area
std::vector<point_t> getEnvPoints(adsr_t *env, area_t area) {
	std::vector<point_t> points;
	// first point is easy enough, just start from the bottom left corner
	points.push_back( { (int16_t) area.x, (int16_t) (area.y + area.h) });
	// figure out how much we should scale stuff relative to the length of the envelope
	const float envMs = totalLengthMs(env);
	float fLength = (float) (envMs - ENV_LENGTH_MIN)
			/ (float) (ENV_LENGTH_MAX - ENV_LENGTH_MIN);
	const int16_t slopedLengthMin = (int16_t) ((1.0f - SUSTAIN_WIDTH_MAX)
			* (float) area.w);
	const int16_t slopedLengthMax = (int16_t) ((1.0f - SUSTAIN_WIDTH_MIN)
			* (float) area.w);
	const int16_t totalSlopedLength = flerp16(slopedLengthMin, slopedLengthMax,
			fLength);
	const int16_t sustainLength = area.w - totalSlopedLength;

	int16_t attackX = (int16_t) ((env->attack / envMs)
			* (float) totalSlopedLength) + area.x;
	// add the top attack point 3px below the top of the component
	const int16_t topMargin = 3;
	points.push_back( { attackX, (int16_t) (area.y + topMargin) });
	// find the y coordinate for the sustain  value
	const int16_t sustainY = (int16_t) flerp16(area.y + topMargin,
			area.y + area.h, 1.0f - env->sustain);
	// find the x for the decay length
	int16_t decayX = attackX
			+ (int16_t) ((env->decay / envMs) * (float) totalSlopedLength);
	points.push_back( { decayX, sustainY });
	int16_t sustainEndX = decayX + sustainLength;
	points.push_back( { sustainEndX, sustainY });

	points.push_back(
			{ (int16_t) (area.x + area.w), (int16_t) (area.y + area.h) });
	return points;
}

std::vector<line_t> getEnvLines(adsr_t *env, area_t area) {
	std::vector<line_t> lines;
	std::vector<point_t> points = getEnvPoints(env, area);
	for (uint8_t i = 1; i < points.size(); i++) {
		lines.push_back( { points[i - 1], points[i] });
	}
	return lines;
}

void drawLineInChunk(line_t line, area_t chunk, uint16_t *buf,
		color16_t lineCol, color16_t bkgndCol) {
	// it's easier to work with the points in terms of left and right
	point_t *left = (line.a.x < line.b.x) ? &line.a : &line.b;
	point_t *right = (left == &line.a) ? &line.b : &line.a;

	// ok step 1 ufortunately there's no way around having to fill the
	// buffer with the background color
	uint16_t xBuf = 0;
	uint16_t yBuf = 0;
	for (xBuf = 0; xBuf < chunk.w; xBuf++) {
		for (yBuf = 0; yBuf < chunk.h; yBuf++) {
			uint16_t *px = getPixel(buf, chunk.w, chunk.h, xBuf, yBuf);
			if (*px != lineCol)
				*px = bkgndCol;
		}
	}
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

//------
EnvGraph::EnvGraph() {

}

void EnvGraph::setParams(adsr_t *p) {
	params = p;
}

void EnvGraph::drawChunk(area_t chunk, uint16_t *buf) {
	std::vector<line_t> lines = getEnvLines(params, area);
	for (auto &line : lines) {
		if (isLineInChunk(line, chunk))
			drawLineInChunk(line, chunk, buf, lineColor, bkgndColor);
	}

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

