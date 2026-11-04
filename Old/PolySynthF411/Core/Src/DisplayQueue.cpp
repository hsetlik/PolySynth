/*
 * DisplayQueue.cpp
 *
 *  Created on: Sep 5, 2024
 *      Author: hayden
 */
#include "DisplayQueue.h"

DisplayQueue::DisplayQueue() {

}

DrawTask DisplayQueue::popFront() {
	if (empty()) {
		return DrawTask();
	}
	DrawTask value = data[tail];
	isFull = false;
	tail = (tail + 1) % DISPLAY_QUEUE_SIZE;
	return value;
}

DrawTask DisplayQueue::peekFront() {
	if (empty()) {
		return DrawTask();
	}
	return data[tail];
}

void DisplayQueue::push(DrawTask obj) {
	data[head] = obj;
	if (isFull) {
		tail = (tail + 1) % DISPLAY_QUEUE_SIZE;
	}
	head = (head + 1) % DISPLAY_QUEUE_SIZE;
	isFull = (head == tail);
}

display_queue_t create_display_queue() {
	return new DisplayQueue();
}
