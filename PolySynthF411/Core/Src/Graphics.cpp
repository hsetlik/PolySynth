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
View::View(){

}

View::~View(){

}

void View::draw(RingBuffer<DrawTask>& queue){
	for(uint8_t i = 0; i < children.size(); i++){
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
	drawStringChunk(font, buf, chunk, text, getStringArea(), txtColor, bkgndColor);
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

float totalLengthMs(adsr_t* env){
	return env->attack + env->decay + env->release;
}

uint16_t flerp16(uint16_t a, uint16_t b, float t){
	return a + (uint16_t)((float)(b - a) * t);
}

// define how short/long the flat section representing sustain can be
// depending on the length of the envelope
#define SUSTAIN_WIDTH_MIN 0.16f
#define SUSTAIN_WIDTH_MAX 0.84f

// line math stuff----------------
bool isLineInChunk(line_t line, area_t area){
	point_t* left = (line.a.x < line.b.x) ? &line.a : &line.b;
	point_t* right = (left == &line.a) ? &line.b : &line.a;
	point_t* upper = (line.a.y < line.b.y) ? &line.a : &line.b;
	point_t* lower = (upper == &line.a) ? &line.b : &line.a;
	area_t lineArea;
	lineArea.x = left->x;
	lineArea.y = upper->y;
	lineArea.w = right->x - left->x;
	lineArea.h = lower->y - upper->y;
	return hasOverlap(area, lineArea);
}

line_t trimLineToArea(line_t line, area_t chunk){
	point_t* left = (line.a.x < line.b.x) ? &line.a : &line.b;
	point_t* right = (left == &line.a) ? &line.b : &line.a;
	point_t* upper = (line.a.y < line.b.y) ? &line.a : &line.b;
	point_t* lower = (upper == &line.a) ? &line.b : &line.a;
	float m = (float)(lower->y - upper->y) / (float)(right->x - left->x);

}
// get the points for a given envelope within a given area
std::vector<point_t> getEnvPoints(adsr_t* env, area_t area) {
	std::vector<point_t> points;
	// first point is easy enough, just start from the bottom left corner
	points.push_back({area.x, (uint16_t)(area.y + area.h)});
	// figure out how much we should scale stuff relative to the length of the envelope
	const float envMs = totalLengthMs(env);
	float fLength = (float)(envMs - ENV_LENGTH_MIN) / (float)(ENV_LENGTH_MAX - ENV_LENGTH_MIN);
	const uint16_t slopedLengthMin = (uint16_t)((1.0f - SUSTAIN_WIDTH_MAX) * (float)area.w);
	const uint16_t slopedLengthMax = (uint16_t)((1.0f - SUSTAIN_WIDTH_MIN) * (float)area.w);
	const uint16_t totalSlopedLength = flerp16(slopedLengthMin, slopedLengthMax, fLength);
	const uint16_t sustainLength = area.w - totalSlopedLength;

	uint16_t attackX = (uint16_t)((env->attack / envMs) * (float)totalSlopedLength) + area.x;
	// add the top attack point 3px below the top of the component
	const uint16_t topMargin = 3;
	points.push_back({attackX, (uint16_t)(area.y + topMargin)});
	// find the y coordinate for the sustain  value
	const uint16_t sustainY = flerp16(area.y + topMargin, area.y + area.h, 1.0f - env->sustain);
	// find the x for the decay length
	uint16_t decayX = attackX + (uint16_t)((env->decay / envMs) * (float)totalSlopedLength);
	points.push_back({decayX, sustainY});
	uint16_t sustainEndX = decayX + sustainLength;
	points.push_back({sustainEndX, sustainY});

	points.push_back({(uint16_t)(area.x + area.w), (uint16_t)(area.y + area.h)});

	return points;
}

//------
EnvGraph::EnvGraph() {

}

void EnvGraph::setParams(adsr_t* p){
	params = p;
}

void EnvGraph::drawChunk(area_t chunk, uint16_t* buf){
	//TODO
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

