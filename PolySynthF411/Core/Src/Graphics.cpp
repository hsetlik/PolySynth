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
		DrawTask task;
		task.area = chunks[c];
		task.func = [this](area_t area, uint16_t *buf) {
			this->drawChunk(area, buf);
		};
		queue->push(task);
	}
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
	float fLevel = (float) currentLevel / (float) maxLevel;
	a.h = (uint16_t) ((1.0f - fLevel) * (float) (area.h - (2 * margin)));
	a.x = area.x + margin;
	a.y = (area.y + area.h) - (a.h + margin);
	a.w = area.w - (2 * margin);
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

	switch (p) {
	case ParamID::pOsc1Coarse:
		int16_t val = (int16_t) params1->coarseTune;
		lCoarseVal1.setText(std::to_string(val));
		gCoarse1.setLevel(val);
		lCoarseVal1.draw();
		gCoarse1.draw();
		break;
	case ParamID::pOsc1Fine:
		int16_t val = (int16_t) params1->fineTune;
		lFineVal1.setText(std::to_string(val));
		gFine1.setLevel(val);
		lFineVal1.draw();
		gFine1.draw();
		break;
	case ParamID::pOsc2Coarse:
		int16_t val = (int16_t) params2->coarseTune;
		lCoarseVal2.setText(std::to_string(val));
		gCoarse2.setLevel(val);
		lCoarseVal2.draw();
		gCoarse2.draw();
		break;
	case ParamID::pOsc2Fine:
		int16_t val = (int16_t) params2->fineTune;
		lFineVal2.setText(std::to_string(val));
		gFine2.setLevel(val);
		lFineVal2.draw();
		gFine2.draw();
		break;
	default:
		break;
	}
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
	env1View.setParams(&patch->envelopes[0]);
	env1View.setName("ADSR 1");
	env2View.setParams(&patch->envelopes[1]);
	env2View.setName("ADSR 2");

	// mixer views
	mix1View.setParams(&patch->oscillators[0]);
	mix1View.setName("DCO 1 Mix");
	mix2View.setParams(&patch->oscillators[1]);
	mix2View.setName("DCO 2 Mix");

	// tuning view
	tuningView.setParams(&patch->oscillators[0], &patch->oscillators[1]);
	tuningView.setName("Tuning");

	views.push_back(&env1View);
	views.push_back(&env2View);
	views.push_back(&mix1View);
	views.push_back(&mix2View);
	views.push_back(&tuningView);

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

void GraphicsProcessor::checkDrawQueue() {
	if (!dmaBusy && !queue->empty()) {
		runFront();
	}
}

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

void GraphicsProcessor::paramUpdated(uint8_t param) {
	View *v = viewForParam(param);
	if (v != nullptr) {
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
			|| (p >= ParamID::pOsc2Coarse && p < ParamID::pOsc2PulseWidth))
		return &tuningView;
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

void check_draw_queue(graphics_processor_t proc) {
	GraphicsProcessor *ptr = static_cast<GraphicsProcessor*>(proc);
	ptr->checkDrawQueue();
}
