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

//------
EnvGraph::EnvGraph() {

}

void EnvGraph::setParams(adsr_t *p) {
	params = p;
}

void EnvGraph::drawChunk(area_t chunk, uint16_t *buf) {
	std::vector<line_t> lines = getEnvLines(params, area);
	fillChunk(buf, chunk.w, chunk.h, bkgndColor);
	for (auto &line : lines) {
		if (isLineInChunk(line, chunk)) {
			drawLineInChunk(line, chunk, buf, lineColor);
		}
	}

}

//VIEWS==================================================
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
EnvView::EnvView() {

}

void EnvView::setParams(adsr_t *p) {
	params = p;
}

void EnvView::initChildren() {
	// 1. set areas for the child components
	const uint16_t labelWidth = ILI9341_WIDTH / 4;
	const uint16_t labelHeight = 15;
	uint16_t lX = 0;
	uint16_t lY = ILI9341_HEIGHT - labelHeight;
	//set the labels
	aLabel.setArea( { lX, lY, labelWidth, labelHeight });
	lX += labelWidth;
	dLabel.setArea( { lX, lY, labelWidth, labelHeight });
	lX += labelWidth;
	sLabel.setArea( { lX, lY, labelWidth, labelHeight });
	lX += labelWidth;
	rLabel.setArea( { lX, lY, labelWidth, labelHeight });

	// set the name label
	const uint16_t nameHeight = 24; // for 11x18 font
	nameLabel.setFont(&Font_11x18);
	nameLabel.setText(viewName);
	nameLabel.setArea( { 0, 0, ILI9341_WIDTH, nameHeight });

	// set the graph
	graph.setArea(
			{ 0, nameHeight, ILI9341_WIDTH, (uint16_t) (lY - nameHeight) });

	// set the label text
	setLabels();

	//2. add everything to the children array
	children.push_back(&aLabel);
	children.push_back(&dLabel);
	children.push_back(&sLabel);
	children.push_back(&rLabel);
	children.push_back(&nameLabel);
	children.push_back(&graph);

}

void EnvView::setLabels() {

	aLabel.setText(textForLabel(&aLabel));
	dLabel.setText(textForLabel(&dLabel));
	sLabel.setText(textForLabel(&sLabel));
	rLabel.setText(textForLabel(&rLabel));

}

std::string EnvView::textForLabel(Label *l) {

	std::string full = "";
	if (l == &aLabel)
		full = "A: " + std::to_string(params->attack);
	else if (l == &dLabel)
		full = "D: " + std::to_string(params->decay);
	else if (l == &sLabel)
		full = "S: " + std::to_string((uint8_t) (params->sustain * 100.0f));
	else
		full = "R: " + std::to_string(params->release);

	// now we need to shorten the strings and add a suffix for each parameter
	if (l == &sLabel) {
		return full.substr(0, 5) + "\%";
	} else {
		return full.substr(0, 6) + "ms";
	}
}

void EnvView::paramUpdated(uint8_t l) {
	uint8_t attackIdx = (uint8_t) ParamID::pEnv1Attack;
	// figure out which envelope this is
	if (l > ParamID::pEnv1Release)
		attackIdx = (uint8_t) ParamID::pEnv2Attack;
	// see the ParamID enum in Patch.h for why this code makes sense
	l = l - attackIdx;
	Label *changedLabel = nullptr;
	switch (l) {
	case 0: // attack
		aLabel.setText(textForLabel(&aLabel));
		changedLabel = &aLabel;
		break;
	case 1: // decay
		dLabel.setText(textForLabel(&dLabel));
		changedLabel = &dLabel;
		break;
	case 2: // sutain
		sLabel.setText(textForLabel(&sLabel));
		changedLabel = &sLabel;
		break;
	case 3: // release
		rLabel.setText(textForLabel(&rLabel));
		changedLabel = &rLabel;
		break;
	default:
		break;
	}

	/** This makes it so that we only redraw the label and the env graph when
	 * a value changes enough to alter the text of the label, the idea being
	 * that a change so small it isn't visible with however many decimal points
	 * on a label is probably also so small that the envelope graph won't be significantly
	 * different and updating it is a waste of time.
	 */

	if (changedLabel->textHasChanged()) {
		changedLabel->draw();
		graph.draw();
	}

}

//MIXER VIEW============================================

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

MixerView::MixerView() {

}

void MixerView::initChildren() {
	// set the name label
	static const uint16_t nameHeight = 24; // for 11x18 font
	nameLabel.setFont(&Font_11x18);
	nameLabel.setText(viewName);
	nameLabel.setArea( { 0, 0, ILI9341_WIDTH, nameHeight });

	static const uint16_t labelWidth = ILI9341_WIDTH / 4;
	static const uint16_t labelHeight = 14; // for 7x10 font
	static const uint16_t valueY = ILI9341_HEIGHT - labelHeight;
	static const uint16_t paramY = valueY - labelHeight;
	static const uint16_t graphHeight = ILI9341_HEIGHT
			- (nameHeight + (labelHeight * 2));
	uint16_t labelX = 0;
	// set up each name label and its corresponding param label
	sawNameLabel.setText("Sawtooth");
	sawNameLabel.setArea( { labelX, paramY, labelWidth, labelHeight });
	sawLevelLabel.setText(std::to_string(params->sawLevel));
	sawLevelLabel.setArea( { labelX, valueY, labelWidth, labelHeight });
	sawLevelGraph.setArea( { labelX, nameHeight, labelWidth, graphHeight });
	sawLevelGraph.setLevel((uint16_t) params->sawLevel);
	labelX += labelWidth;

	triNameLabel.setText("Triangle");
	triNameLabel.setArea( { labelX, paramY, labelWidth, labelHeight });
	triLevelLabel.setText(std::to_string(params->triLevel));
	triLevelLabel.setArea( { labelX, valueY, labelWidth, labelHeight });
	triLevelGraph.setArea( { labelX, nameHeight, labelWidth, graphHeight });
	triLevelGraph.setLevel((uint16_t) params->triLevel);
	labelX += labelWidth;

	pulseNameLabel.setText("Pulse");
	pulseNameLabel.setArea( { labelX, paramY, labelWidth, labelHeight });
	pulseLevelLabel.setText(std::to_string(params->pulseLevel));
	pulseLevelLabel.setArea( { labelX, valueY, labelWidth, labelHeight });
	pulseLevelGraph.setArea( { labelX, nameHeight, labelWidth, graphHeight });
	pulseLevelGraph.setLevel((uint16_t) params->pulseLevel);
	labelX += labelWidth;

	masterNameLabel.setText("Master");
	masterNameLabel.setArea( { labelX, paramY, labelWidth, labelHeight });
	masterLevelLabel.setText(std::to_string(params->oscLevel));
	masterLevelLabel.setArea( { labelX, valueY, labelWidth, labelHeight });
	masterLevelGraph.setArea( { labelX, nameHeight, labelWidth, graphHeight });
	masterLevelGraph.setLevel((uint16_t) params->oscLevel);
	labelX += labelWidth;

	children.push_back(&nameLabel);
	children.push_back(&sawNameLabel);
	children.push_back(&sawLevelLabel);
	children.push_back(&sawLevelGraph);
	children.push_back(&triNameLabel);
	children.push_back(&triLevelLabel);
	children.push_back(&triLevelGraph);
	children.push_back(&pulseNameLabel);
	children.push_back(&pulseLevelLabel);
	children.push_back(&pulseLevelGraph);
	children.push_back(&masterNameLabel);
	children.push_back(&masterLevelLabel);
	children.push_back(&masterLevelGraph);
}

void MixerView::paramUpdated(uint8_t id) {
	uint8_t squareIdx = (uint8_t) ParamID::pOsc1SquareLevel;
	if (id > (uint8_t) ParamID::pOsc1OscLevel
			&& id <= (uint8_t) ParamID::pOsc2OscLevel) {
		squareIdx = (uint8_t) ParamID::pOsc2SquareLevel;
	}
	//determine which param
	id -= squareIdx;
	switch (id) {
	case 0: // pulse
		pulseLevelLabel.setText(std::to_string(params->pulseLevel));
		pulseLevelLabel.draw();
		pulseLevelGraph.setLevel((uint16_t) params->pulseLevel);
		pulseLevelGraph.draw();
		break;
	case 1: // saw
		sawLevelLabel.setText(std::to_string(params->sawLevel));
		sawLevelLabel.draw();
		sawLevelGraph.setLevel((uint16_t) params->sawLevel);
		sawLevelGraph.draw();
		break;
	case 2: // tri
		triLevelLabel.setText(std::to_string(params->triLevel));
		triLevelLabel.draw();
		triLevelGraph.setLevel((uint16_t) params->triLevel);
		triLevelGraph.draw();
		break;
	case 3: // master
		masterLevelLabel.setText(std::to_string(params->oscLevel));
		masterLevelLabel.draw();
		masterLevelGraph.setLevel((uint16_t) params->oscLevel);
		masterLevelGraph.draw();
		break;
	default:
		break;
	}
}

// TUNING VIEW=============================================
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

OscTuningView::OscTuningView() {

}

void OscTuningView::initChildren() {
	// set the name label
	static const uint16_t nameHeight = 24; // for 11x18 font
	nameLabel.setFont(&Font_11x18);
	nameLabel.setText(viewName);
	nameLabel.setArea( { 0, 0, ILI9341_WIDTH, nameHeight });

	static const uint16_t sectionW = ILI9341_WIDTH / 4;

	// set up each osc's name label
	lOsc1.setFont(&Font_11x18);
	lOsc1.setText("DCO 1");
	lOsc1.setArea( { 0, nameHeight, (uint16_t) (2 * sectionW), nameHeight });

	lOsc2.setFont(&Font_11x18);
	lOsc2.setText("DCO 2");
	lOsc2.setArea(
			{ (uint16_t) (2 * sectionW), nameHeight, (uint16_t) (2 * sectionW),
					nameHeight });

	// now the labels below each
	uint16_t currentX = 0;
	static const uint16_t labelHeight = 14;
	static const uint16_t valueY = ILI9341_HEIGHT - labelHeight;
	static const uint16_t nameY = valueY - labelHeight;
	static const uint16_t graphY = 2 * nameHeight;
	static const uint16_t graphHeight = nameY - graphY;

	// coarse tune 1
	gCoarse1.setArea( { currentX, graphY, sectionW, graphHeight });
	lCoarseName1.setArea( { currentX, nameY, sectionW, labelHeight });
	lCoarseVal1.setArea( { currentX, valueY, sectionW, labelHeight });
	gCoarse1.setLevel((int16_t) params1->coarseTune);
	lCoarseName1.setText("COARSE");
	lCoarseVal1.setText(std::to_string(params1->coarseTune));
	currentX += sectionW;

	// fine tune 1
	gFine1.setArea( { currentX, graphY, sectionW, graphHeight });
	lFineName1.setArea( { currentX, nameY, sectionW, labelHeight });
	lFineVal1.setArea( { currentX, valueY, sectionW, labelHeight });
	gFine1.setLevel((int16_t) params1->fineTune);
	lFineName1.setText("COARSE");
	lFineVal1.setText(std::to_string(params1->fineTune));
	currentX += sectionW;

	// coarse tune 2
	gCoarse2.setArea( { currentX, graphY, sectionW, graphHeight });
	lCoarseName2.setArea( { currentX, nameY, sectionW, labelHeight });
	lCoarseVal2.setArea( { currentX, valueY, sectionW, labelHeight });
	gCoarse2.setLevel((int16_t) params2->coarseTune);
	lCoarseName2.setText("COARSE");
	lCoarseVal2.setText(std::to_string(params2->coarseTune));
	currentX += sectionW;

	// fine tune 2
	gFine2.setArea( { currentX, graphY, sectionW, graphHeight });
	lFineName2.setArea( { currentX, nameY, sectionW, labelHeight });
	lFineVal2.setArea( { currentX, valueY, sectionW, labelHeight });
	gFine2.setLevel((int16_t) params2->fineTune);
	lFineName2.setText("COARSE");
	lFineVal2.setText(std::to_string(params2->fineTune));

	// now add pointers to the children vector
	children.push_back(&nameLabel);

	children.push_back(&lOsc1);
	children.push_back(&lFineName1);
	children.push_back(&lFineVal1);
	children.push_back(&gFine1);
	children.push_back(&lCoarseName1);
	children.push_back(&lCoarseVal1);
	children.push_back(&gCoarse1);

	children.push_back(&lOsc2);
	children.push_back(&lFineName2);
	children.push_back(&lFineVal2);
	children.push_back(&gFine2);
	children.push_back(&lCoarseName2);
	children.push_back(&lCoarseVal2);
	children.push_back(&gCoarse2);

}

void OscTuningView::paramUpdated(uint8_t id) {
	ParamID p = (ParamID) id;
	int16_t val = 0;
	switch (p) {
	case ParamID::pOsc1Coarse:
		val = (int16_t) params1->coarseTune;
		lCoarseVal1.setText(std::to_string(val));
		gCoarse1.setLevel(val);
		lCoarseVal1.draw();
		gCoarse1.draw();
		break;
	case ParamID::pOsc1Fine:
		val = (int16_t) params1->fineTune;
		lFineVal1.setText(std::to_string(val));
		gFine1.setLevel(val);
		lFineVal1.draw();
		gFine1.draw();
		break;
	case ParamID::pOsc2Coarse:
		val = (int16_t) params2->coarseTune;
		lCoarseVal2.setText(std::to_string(val));
		gCoarse2.setLevel(val);
		lCoarseVal2.draw();
		gCoarse2.draw();
		break;
	case ParamID::pOsc2Fine:
		val = (int16_t) params2->fineTune;
		lFineVal2.setText(std::to_string(val));
		gFine2.setLevel(val);
		lFineVal2.draw();
		gFine2.draw();
		break;
	default:
		break;
	}
}

// LFO VIEW=======================================
// graph
GraphBuffer::GraphBuffer(){

}

void GraphBuffer::push(uint16_t val){
	data[head] = val;
	head = (head + 1) % LFO_GRAPH_POINTS;
}


LFOGraph::LFOGraph(){

}

void LFOGraph::updateGraphLines(area_t area) {
	int16_t x0 = (int16_t)area.x;
	const int16_t dX = (int16_t)((float)area.w / (float)LFO_GRAPH_POINTS);
	const float yScale = (float)area.h / 4096.0f;
	const int16_t yBottom = (int16_t)area.y + area.h;
	int16_t y0 = yBottom - (int16_t)((float)lfoPoints[0] * yScale);
	int16_t x1, y1;
	for(uint8_t l = 1; l < LFO_GRAPH_POINTS; l++){
		x1 = x0 + dX;
		y1 = yBottom - (int16_t)((float)lfoPoints[l] * yScale);
		graphLines[l - 1] = {{x0, y0}, {x1, y1}};
		x0 = x1;
		y0 = y1;
	}
}

bool LFOGraph::needsGraphPoint(){
	static tick_t now = 0;
	now = TickTimer_get();
	// check if the frequency has changed
	static float prevFreq = 1.0f;
	static float periodMs = 1000.0f / prevFreq;
	if(prevFreq != params->freq){
		prevFreq = params->freq;
		periodMs = 1000.0f / prevFreq;
	}
	return TickTimer_tickDistanceMs(lastGraphPoint, now) > periodMs;
}

void LFOGraph::addGraphPoint(uint16_t val){
	lfoPoints.push(val);
	lastGraphPoint = TickTimer_get();
}



void LFOGraph::drawChunk(area_t chunk, uint16_t *buf){
	//TODO
}



//MODAL VIEW===========================================
// base comp----------------------------------------
BaseChangeComponent::BaseChangeComponent() {
	lName.bkgndColor = bkgndColor;
	lValue.bkgndColor = bkgndColor;
	children.push_back(&lName);
	children.push_back(&lValue);
	children.push_back(&graph);
}

void BaseChangeComponent::drawChunk(area_t chunk, uint16_t *buf) {
	// first fill with the background/edge color as needed
	uint16_t *px = nullptr;
	for (uint16_t x = chunk.x; x < (chunk.x + chunk.w); x++) {
		for (uint16_t y = chunk.y; y < (chunk.y + chunk.h); y++) {
			px = getPixel(buf, chunk.w, chunk.h, x - chunk.x, y - chunk.y);
			// check if we're on the margin
			if ((x < margin) || (x >= (area.x + area.w) - margin)
					|| (y < margin) || (y >= (area.y + area.h))) {
				*px = (uint16_t) edgeColor;
			} else {
				*px = (uint16_t) bkgndColor;
			}

		}
	}
	// now draw any relevant child components
	for (auto *child : children) {
		if (hasOverlap(chunk, child->getArea()))
			child->drawChunk(chunk, buf);
	}

}

void BaseChangeComponent::placeChildren() {
	// little idea maybe
	const uint16_t dX = area.w / 12;
	const uint16_t dY = area.h / 9;

	area_t nameArea;
	nameArea.x = dX;
	nameArea.y = dY;
	nameArea.w = dX * 10;
	nameArea.h = lName.getIdealHeight(2);
	lName.setArea(nameArea);

	area_t valueArea;
	valueArea.x = dX;
	valueArea.y = 2 * dY;
	valueArea.w = dX * 10;
	valueArea.h = lValue.getIdealHeight(2);
	lValue.setArea(valueArea);

	area_t graphArea;
	graphArea.x = 2 * dX;
	graphArea.y = 4 * dY;
	graphArea.w = dX * 8;
	graphArea.h = dY * 2;
	graph.setArea(graphArea);

}

void BaseChangeComponent::prepareToShow(const std::string &name,
		const std::string &value, int16_t level, int16_t maxLevel) {
	lName.setText(name);
	lValue.setText(value);
	graph.setMaxLevel(maxLevel);
	graph.setLevel(level);
	//NOTE: we don't always want the bar graph and we can indicate that by passing a maxLevel < 0
	if (maxLevel < 0)
		forbiddenComp = &graph;
	else
		forbiddenComp = nullptr;
}

// mod comp----------------------------------------
ModChangeComponent::ModChangeComponent() {
	// set up the header component
	lHeader.setFont(&Font_11x18);
	lHeader.setText("Selected Modulation:");
	graph.setMaxLevel(128);
	children.push_back(&lHeader);
	children.push_back(&lName);
	children.push_back(&lValue);
	children.push_back(&graph);
}

void ModChangeComponent::placeChildren() {

	const uint16_t dX = area.w / 12;
	const uint16_t dY = area.h / 9;

	area_t headerArea;
	headerArea.x = 2 * dX;
	headerArea.y = dY;
	headerArea.w = dX * 10;
	headerArea.h = lHeader.getIdealHeight(2);
	lHeader.setArea(headerArea);

	area_t nameArea;
	nameArea.x = dX;
	nameArea.y = 3 * dY;
	nameArea.w = dX * 10;
	nameArea.h = lName.getIdealHeight(2);
	lName.setArea(nameArea);

	area_t valueArea;
	valueArea.x = dX;
	valueArea.y = 4 * dY;
	valueArea.w = dX * 10;
	valueArea.h = lValue.getIdealHeight(2);
	lValue.setArea(valueArea);

	area_t graphArea;
	graphArea.x = 2 * dX;
	graphArea.y = 6 * dY;
	graphArea.w = dX * 8;
	graphArea.h = dY * 2;
	graph.setArea(graphArea);
}

// helpers for setting the label text
std::string modSourceString(uint8_t id) {
	switch (id) {
	case ModSource::ENV1:
		return "ADSR 1";
	case ModSource::ENV2:
		return "ADSR 2";
	case ModSource::LFO1:
		return "LFO 1";
	case ModSource::LFO2:
		return "LFO 2";
	case ModSource::LFO3:
		return "LFO 3";
	case ModSource::MODWHL:
		return "Mod Wheel";
	case ModSource::PITCHWHL:
		return "Pitch Wheel";
	case ModSource::VEL:
		return "Note Velocity";
	default:
		return "NONE";
	}
	return "";
}

std::string modDestString(uint8_t id) {
	switch (id) {
	case ModDest::CUTOFF:
		return "Filter Cutoff";
	case ModDest::RESONANCE:
		return "Filter Resonance";
	case ModDest::FOLD:
		return "Wavefolder Amount";
	case ModDest::PWM1:
		return "Osc 1 PWM";
	case ModDest::PWM2:
		return "Osc 2 PWM";
	case ModDest::TUNE1:
		return "Osc 1 Detune";
	case ModDest::TUNE2:
		return "Osc 2 Detune";
	case ModDest::VCA:
		return "VCA";
	default:
		return "null";
	}

}

void ModChangeComponent::prepareToShow(uint8_t source, uint8_t dest,
		int8_t depth) {
	std::string val = modSourceString(source) + " >> " + modDestString(dest);
	lName.setText(val);
	lValue.setText(std::to_string(depth));
	graph.setLevel(depth);

}

void ModChangeComponent::drawChunk(area_t chunk, uint16_t *buf) {

	uint16_t *px = nullptr;
	for (uint16_t x = chunk.x; x < (chunk.x + chunk.w); x++) {
		for (uint16_t y = chunk.y; y < (chunk.y + chunk.h); y++) {
			px = getPixel(buf, chunk.w, chunk.h, x - chunk.x, y - chunk.y);
			// check if we're on the margin
			if ((x < margin) || (x >= (area.x + area.w) - margin)
					|| (y < margin) || (y >= (area.y + area.h))) {
				*px = (uint16_t) edgeColor;
			} else {
				*px = (uint16_t) bkgndColor;
			}

		}
	}
	// now draw any relevant child components
	for (auto *child : children) {
		if (hasOverlap(chunk, child->getArea()))
			child->drawChunk(chunk, buf);
	}
}

// view -------------------
ModalChangeView::ModalChangeView() {

}

void ModalChangeView::initChildren() {
	constexpr uint16_t dX = ILI9341_WIDTH / 8;
	constexpr uint16_t dY = ILI9341_HEIGHT / 8;
	area_t modalArea;
	modalArea.x = dX;
	modalArea.y = dY;
	modalArea.w = 6 * dX;
	modalArea.h = 6 * dY;

	baseModal.setArea(modalArea);
	baseModal.placeChildren();
	modModal.setArea(modalArea);
	modModal.placeChildren();

	children.push_back(&baseModal);
	//children.push_back(&modModal);
}

void ModalChangeView::paramUpdated(uint8_t id) {
	lastUpdateTick = TickTimer_get();
	/** COMPLETE LIST OF PARAMS THIS NEEDS TO HANDE (as of 9/7):
	 * - Pulse width 1: value/4096
	 * - Pulse width 2
	 * - Filter cutoff
	 * - Filter resonance
	 * - Filter mode (hi/low pass)
	 * - Fold level
	 * - Fold Order (whether filter -> folder or folder -> filter)
	 */
	std::string nameStr, valStr;
	switch (id) {
	case ParamID::pOsc1PulseWidth:
		nameStr = "DCO 1 Pulse Width";
		valStr = std::to_string(patch->oscs[0].pulseWidth) + "/4096";
		baseModal.prepareToShow(nameStr, valStr,
				patch->oscs[0].pulseWidth, 4096);
		break;
	case ParamID::pOsc2PulseWidth:
		nameStr = "DCO 2 Pulse Width";
		valStr = std::to_string(patch->oscs[1].pulseWidth) + "/4096";
		baseModal.prepareToShow(nameStr, valStr,
				patch->oscs[1].pulseWidth, 4096);
		break;
	case ParamID::pFilterCutoff:
		nameStr = "Filter Cutoff";
		valStr = std::to_string(patch->cutoffBase) + "/4096";
		baseModal.prepareToShow(nameStr, valStr, patch->cutoffBase, 4096);
		break;
	case ParamID::pFilterRes:
		nameStr = "Filter Resonance";
		valStr = std::to_string(patch->resBase) + "/256";
		baseModal.prepareToShow(nameStr, valStr, patch->resBase, 256);
		break;
	case ParamID::pFilterMode:
		nameStr = "Filter Mode";
		valStr = (patch->highPassMode > 0) ? "High Pass" : "Low Pass";
		// the -1 indicates that we won't show the graph for this
		baseModal.prepareToShow(nameStr, valStr, 0, -1);
		break;
	case ParamID::pFoldLevel:
		nameStr = "Wavefolder Level";
		valStr = std::to_string(patch->foldBase) + "/4096";
		baseModal.prepareToShow(nameStr, valStr, patch->foldBase, 4096);
		break;
	case ParamID::pFoldFirst:
		nameStr = "Wavefolder/Filter Order";
		valStr =
				(patch->foldFirst > 0) ?
						"Wavefolder > Filter" : "Filter > Wavefolder";
		baseModal.prepareToShow(nameStr, valStr, 0, -1);
		break;
	default:
		break;
	}
	baseModal.draw();
}

bool ModalChangeView::timeToRemove() {
	constexpr float modalLengthMs = 3100.0f;
	return TickTimer_tickDistanceMs(lastUpdateTick, TickTimer_get())
			>= modalLengthMs;
}


//======================================================

GraphicsProcessor::GraphicsProcessor() :
		queue(static_cast<DisplayQueue*>(mainDispQueue)), dmaBusy(false) {
// initialize the screen
	ILI9341_Init();

}

GraphicsProcessor::~GraphicsProcessor() {

}

void GraphicsProcessor::initViews() {

	// envelope views
	env1View.setParams(&patch->envs[0]);
	env1View.setName("ADSR 1");
	env2View.setParams(&patch->envs[1]);
	env2View.setName("ADSR 2");

	// mixer views
	mix1View.setParams(&patch->oscs[0]);
	mix1View.setName("DCO 1 Mix");
	mix2View.setParams(&patch->oscs[1]);
	mix2View.setName("DCO 2 Mix");

	// tuning view
	tuningView.setParams(&patch->oscs[0], &patch->oscs[1]);
	tuningView.setName("Tuning");

	// modal view
	modalView.setPatch(patch);

	views.push_back(&env1View);
	views.push_back(&env2View);
	views.push_back(&mix1View);
	views.push_back(&mix2View);
	views.push_back(&tuningView);
	views.push_back(&modalView);

	// initialize all the views' child components
	for (View *v : views) {
		v->initChildren();
	}

	// make envelope 1 visible to start
	visibleView = &env1View;
	visibleView->draw();
}

void GraphicsProcessor::dmaFinished() {
	dmaBusy = false;
}

void GraphicsProcessor::checkGUIUpdates() {
	if (inModalMode && modalView.timeToRemove()) {
		undrawModal();
		inModalMode = false;
	}
	if (!dmaBusy && !queue->empty()) {
		runFront();
	}
}


View* GraphicsProcessor::viewForID(uint8_t idx){
	ViewID id = (ViewID)idx;
	switch(id){
	case vEnv1:
		return &env1View;
	case vEnv2:
		return &env2View;
	case vMix1:
		return &mix1View;
	case vMix2:
		return &mix2View;
	case vTune:
		return &tuningView;
	default:
		return nullptr;
	}
}

void GraphicsProcessor::selectView(uint8_t idx){
	View* v = viewForID(idx);
	if(v != nullptr && v != visibleView){
		visibleView = v;
		visibleView->draw();
	}

}
//----------------------------------------

void GraphicsProcessor::pushTask(DrawTask task) {
	queue->push(task);
}

void GraphicsProcessor::runFront() {
	DrawTask t = queue->popFront();
// run the callback to render the pixels
	t.func(t.area, dmaBuf);
	ILI9341_DrawImage_DMA(t.area.x, t.area.y, t.area.w, t.area.h, dmaBuf);
	dmaBusy = true;

}

void GraphicsProcessor::undrawModal() {
	auto chunks = modalView.getChunksForArea();
	// bc our visibleView* doesn't change when we enter modal mode
	visibleView->redrawChunks(chunks);
}

void GraphicsProcessor::paramUpdated(uint8_t param) {
	View *v = viewForParam(param);
	if (v != nullptr) {
		inModalMode = (v == &modalView);
		v->paramUpdated(param);
	}
}

View* GraphicsProcessor::viewForParam(uint8_t p) {
	// refer to the ParamID enum in Patch.h for my reasoning here
	if (p < ParamID::pEnv2Attack) { // envelopes--------------
		return &env1View;
	} else if (p >= ParamID::pEnv2Attack && p < ParamID::pLFO1Freq) {
		return &env2View;
	} else if (p >= ParamID::pOsc1SquareLevel && p < ParamID::pOsc2Coarse) {
		return &mix1View;
	} else if (p >= ParamID::pOsc2SquareLevel && p < ParamID::pFilterCutoff) {
		return &mix2View;
	} else if ((p >= ParamID::pOsc1Coarse && p < ParamID::pOsc1PulseWidth)
			|| (p >= ParamID::pOsc2Coarse && p < ParamID::pOsc2PulseWidth)) {
		return &tuningView;
	} else if ((p == ParamID::pOsc1PulseWidth || p == ParamID::pOsc2PulseWidth)
			|| (p >= ParamID::pFilterCutoff)) {
		return &modalView;
	}
	return nullptr;
}
//======================================================

graphics_processor_t create_graphics_processor() {
	return new GraphicsProcessor();
}

void disp_dma_finished(graphics_processor_t proc) {
	GraphicsProcessor *ptr = static_cast<GraphicsProcessor*>(proc);
	ptr->dmaFinished();
}

void check_gui_updates(graphics_processor_t proc) {
	GraphicsProcessor *ptr = static_cast<GraphicsProcessor*>(proc);
	ptr->checkGUIUpdates();
}
