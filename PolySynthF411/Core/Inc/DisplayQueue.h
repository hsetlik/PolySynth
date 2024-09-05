/*
 * DisplayQueue.h
 *
 *  Created on: Sep 5, 2024
 *      Author: hayden
 */

#ifndef INC_DISPLAYQUEUE_H_
#define INC_DISPLAYQUEUE_H_


#ifdef __cplusplus
#include "main.h"
#include "ili9341.h"
#include <functional>

#define DISPLAY_QUEUE_SIZE 50

// each GUI element will be split up into some number of DrawTasks

typedef std::function<void(area_t a, uint16_t*)> DrawFunc;

struct DrawTask {
	DrawFunc func;
	area_t area;
};

class DisplayQueue { // simple circular buffer implementation
private:
	DrawTask data[DISPLAY_QUEUE_SIZE];
	uint16_t head = 0;
	uint16_t tail = 0;
	bool isFull = false;
public:
	DisplayQueue();
	DrawTask popFront();
	DrawTask peekFront();
	void push(DrawTask obj);
	bool full() {
		return isFull;
	}
	bool empty(){
		return(!isFull && (head == tail));
	}
};







#endif //__cplusplus

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

//Note: the c-friendly typedef for the queue is in main.h
EXTERNC display_queue_t create_display_queue();

#undef EXTERNC

#endif /* INC_DISPLAYQUEUE_H_ */
