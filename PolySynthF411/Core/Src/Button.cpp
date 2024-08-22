/*
 * Button.cpp
 *
 *  Created on: Aug 21, 2024
 *      Author: hayden
 */
#include "Button.h"

// helpers to untangle the addr/port/pin business
button_t buttonForID(uint8_t idx) {
	button_t b;
	ButtonID id = (ButtonID) idx;
	switch (id) {
	case Alt:
		b.addr = B_ALT_ADDR;
		b.port = B_ALT_PORT;
		b.pin = B_ALT_PIN;
		break;
	case Env1:
		b.addr = B_ENV1_ADDR;
		b.port = B_ENV1_PORT;
		b.pin = B_ENV1_PIN;
		break;
	case Env2:
		b.addr = B_ENV2_ADDR;
		b.port = B_ENV2_PORT;
		b.pin = B_ENV2_PIN;
		break;
	case LFO1B:
		b.addr = B_LFO1_ADDR;
		b.port = B_LFO1_PORT;
		b.pin = B_LFO1_PIN;
		break;
	case LFO2B:
		b.addr = B_LFO2_ADDR;
		b.port = B_LFO2_PORT;
		b.pin = B_LFO2_PIN;
		break;
	case LFO3B:
		b.addr = B_LFO3_ADDR;
		b.port = B_LFO3_PORT;
		b.pin = B_LFO3_PIN;
		break;
	case Osc1:
		b.addr = B_OSC1_ADDR;
		b.port = B_OSC1_PORT;
		b.pin = B_OSC1_PIN;
		break;
	case Osc2:
		b.addr = B_OSC2_ADDR;
		b.port = B_OSC2_PORT;
		b.pin = B_OSC2_PIN;
		break;
	case PWMB:
		b.addr = B_PWM_ADDR;
		b.port = B_PWM_PORT;
		b.pin = B_PWM_PIN;
		break;
	case FilterMode:
		b.addr = B_FILTERMODE_ADDR;
		b.port = B_FILTERMODE_PORT;
		b.pin = B_FILTERMODE_PIN;
		break;
	case FoldFirst:
		b.addr = B_FOLDFIRST_ADDR;
		b.port = B_FOLDFIRST_PORT;
		b.pin = B_FOLDFIRST_PIN;
		break;
	case Menu:
		b.addr = B_MENU_ADDR;
		b.port = B_MENU_PORT;
		b.pin = B_MENU_PIN;
		break;
	case MS1:
		b.addr = B_MS1_ADDR;
		b.port = B_MS1_PORT;
		b.pin = B_MS1_PIN;
		break;
	case MS2:
		b.addr = B_MS2_ADDR;
		b.port = B_MS2_PORT;
		b.pin = B_MS2_PIN;
		break;
	case MS3:
		b.addr = B_MS3_ADDR;
		b.port = B_MS3_PORT;
		b.pin = B_MS3_PIN;
		break;
	case MS4:
		b.addr = B_MS4_ADDR;
		b.port = B_MS4_PORT;
		b.pin = B_MS4_PIN;
		break;
	case MS5:
		b.addr = B_MS5_ADDR;
		b.port = B_MS5_PORT;
		b.pin = B_MS5_PIN;
		break;
	case MS6:
		b.addr = B_MS6_ADDR;
		b.port = B_MS6_PORT;
		b.pin = B_MS6_PIN;
		break;
	case ModBank:
		b.addr = B_MODBANK_ADDR;
		b.port = B_MODBANK_PORT;
		b.pin = B_MODBANK_PIN;
		break;
	case MD1:
		b.addr = B_MD1_ADDR;
		b.port = B_MD1_PORT;
		b.pin = B_MD1_PIN;
		break;
	case MD2:
		b.addr = B_MD2_ADDR;
		b.port = B_MD2_PORT;
		b.pin = B_MD2_PIN;
		break;
	case MD3:
		b.addr = B_MD3_ADDR;
		b.port = B_MD3_PORT;
		b.pin = B_MD3_PIN;
		break;
	case MD4:
		b.addr = B_MD4_ADDR;
		b.port = B_MD4_PORT;
		b.pin = B_MD4_PIN;
		break;
	case MD5:
		b.addr = B_MD5_ADDR;
		b.port = B_MD5_PORT;
		b.pin = B_MD5_PIN;
		break;
	case MD6:
		b.addr = B_MD6_ADDR;
		b.port = B_MD6_PORT;
		b.pin = B_MD6_PIN;
		break;
	case MD7:
		b.addr = B_MD7_ADDR;
		b.port = B_MD7_PORT;
		b.pin = B_MD7_PIN;
		break;
	case MD8:
		b.addr = B_MD8_ADDR;
		b.port = B_MD8_PORT;
		b.pin = B_MD8_PIN;
		break;
	default:
		break;
	}
	return b;
}

uint8_t buttonForPin(uint8_t addr, uint8_t port, uint8_t pin) {
	// 1. which expander are we on?
	switch (addr) {
	case EXP_1_ADDR:
		if (port) { // MD buttons
			switch (pin) {
			case B_MD1_PIN:
				return (uint8_t) ButtonID::MD1;
			case B_MD2_PIN:
				return (uint8_t) ButtonID::MD2;
			case B_MD3_PIN:
				return (uint8_t) ButtonID::MD3;
			case B_MD4_PIN:
				return (uint8_t) ButtonID::MD4;
			case B_MD5_PIN:
				return (uint8_t) ButtonID::MD5;
			case B_MD6_PIN:
				return (uint8_t) ButtonID::MD6;
			case B_MD7_PIN:
				return (uint8_t) ButtonID::MD7;
			case B_MD8_PIN:
				return (uint8_t) ButtonID::MD8;
			default:
				break;
			}
		} else { // MS and ModBank buttons
			switch (pin) {
			case B_MS1_PIN:
				return (uint8_t) ButtonID::MS1;
			case B_MS2_PIN:
				return (uint8_t) ButtonID::MS2;
			case B_MS3_PIN:
				return (uint8_t) ButtonID::MS3;
			case B_MS4_PIN:
				return (uint8_t) ButtonID::MS4;
			case B_MS5_PIN:
				return (uint8_t) ButtonID::MS5;
			case B_MS6_PIN:
				return (uint8_t) ButtonID::MS6;
			case B_MODBANK_PIN:
				return (uint8_t) ButtonID::ModBank;
			}

		}
		break;
	case EXP_2_ADDR: // we don't need to check the port bc the only buttons are on port B
		switch (pin) {
		case B_MENU_PIN:
			return (uint8_t) ButtonID::Menu;
		case B_PWM_PIN:
			return (uint8_t) ButtonID::PWMB;
		case B_FILTERMODE_PIN:
			return (uint8_t) ButtonID::FilterMode;
		case B_FOLDFIRST_PIN:
			return (uint8_t) ButtonID::FoldFirst;
		default:
			break;
		}
		break;
	case EXP_3_ADDR: // again no need to check port bc all the buttons are on port B
		switch (pin) {
		case B_ALT_PIN:
			return (uint8_t) ButtonID::Alt;
		case B_ENV1_PIN:
			return (uint8_t) ButtonID::Env1;
		case B_ENV2_PIN:
			return (uint8_t) ButtonID::Env2;
		case B_LFO1_PIN:
			return (uint8_t) ButtonID::LFO1B;
		case B_LFO2_PIN:
			return (uint8_t) ButtonID::LFO2B;
		case B_LFO3_PIN:
			return (uint8_t) ButtonID::LFO3B;
		case B_OSC1_PIN:
			return (uint8_t) ButtonID::Osc1;
		case B_OSC2_PIN:
			return (uint8_t) ButtonID::Osc2;
		default:
			break;
		}
		break;
	}
	return 0;
}

//===================================================================

Btn::Btn() :
		activeLevel(false), state(INIT) {

}

void Btn::tick(bool lvl) {
	fsm(debounce(lvl) == activeLevel);
}

void Btn::reset() {
	state = BtnState::INIT;
	numClicks = 0;
	startTime = TickTimer_get();

}

bool Btn::debounce(bool lvl) {
	now = TickTimer_get();
	if (lastDebounceLevel == lvl) {
		float diff = TickTimer_tickDistanceMs(lastDebounceTime, now);
		if (diff > DEBOUNCE_MS)
			debouncedLevel = lvl;
	} else {
		lastDebounceTime = now;
		lastDebounceLevel = lvl;
	}
	return debouncedLevel;
}

void Btn::fsm(bool lvl) {
	float waitTime = TickTimer_tickDistanceMs(startTime, now);
	switch (state) {
	case INIT:
		if (lvl) {
			state = BtnState::DOWN;
			startTime = now;
			numClicks = 0;
		}
		break;
	case DOWN:
		if (!lvl) {
			state = BtnState::UP;
			startTime = now;
		} else if (waitTime >= PRESS_MS) {
			if (hasOnPressStart)
				onPressStart();
			state = BtnState::PRESS;
		}
		break;
	case UP:
		numClicks++;
		state = BtnState::COUNT;
		break;
	case COUNT:
		if (lvl) {
			state = BtnState::DOWN;
			startTime = now;
		} else if ((waitTime >= CLICK_MS) || numClicks == maxClicks) {
			if (numClicks == 1) {
				if (hasOnClick)
					onClick();
			} else if (numClicks == 1) {
				//TODO: onDoubleClick runs here
			}
			reset();
		}
		break;
	case PRESS:
		if (!lvl) {
			state = BtnState::PRESSEND;
		} else {
			if (TickTimer_tickDistanceMs(lastDuringPressTime, now) >= PRESS_INTERVAL) {
				if (hasDuringPress)
					duringPress();
				lastDuringPressTime = now;
			}
		}
		break;
	case PRESSEND:
		if (hasOnPressEnd)
			onPressEnd();
		reset();
		break;
	default:
		state = BtnState::INIT;
		break;
	}
}

//===========================================================

ButtonProcessor::ButtonProcessor() {

	for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
		// 1. initialize locations
		// 2. set up function pointers
		locations[i] = buttonForID(i);

		BtnCallback onClick = [this, i]() {
			if (this->onClickHandler != nullptr)
				onClickHandler(i);
		};
		buttons[i].setOnClick(onClick);

		BtnCallback onPressStart = [this, i]() {
			if (this->onPressStartHandler != nullptr)
				onPressStartHandler(i);
		};
		buttons[i].setOnPressStart(onPressStart);

		BtnCallback duringPress = [this, i]() {
			if (this->duringPressHandler != nullptr)
				duringPressHandler(i);
		};
		buttons[i].setDuringPress(duringPress);

		BtnCallback onPressEnd = [this, i]() {
			if (this->onPressEndHandler != nullptr)
				onPressEndHandler(i);
		};
		buttons[i].setOnPressEnd(onPressEnd);
	}
}

void ButtonProcessor::checkButtons(){
	// step 1: grip the 4 bytes of data we need via SPI
	uint8_t exp1A = MCP23S17_getGPIOBits(&hspi2, EXP_1_ADDR, 0);
	uint8_t exp1B = MCP23S17_getGPIOBits(&hspi2, EXP_1_ADDR, 1);
	uint8_t exp2B = MCP23S17_getGPIOBits(&hspi2, EXP_2_ADDR, 1);
	uint8_t exp3B = MCP23S17_getGPIOBits(&hspi2, EXP_3_ADDR, 1);

	// step 2: find the correct bit for each button
	for(uint8_t b = 0; b < NUM_BUTTONS; b++){
		switch(locations[b].addr){
		case EXP_1_ADDR:
			if(locations[b].port)
				buttons[b].tick(exp1B & (1 << locations[b].pin));
			else
				buttons[b].tick(exp1A & (1 << locations[b].pin));
			break;
		case EXP_2_ADDR:
			buttons[b].tick(exp2B & (1 << locations[b].pin));
		case EXP_3_ADDR:
			buttons[b].tick(exp3B & (1 << locations[b].pin));
		default:
			break;
		}
	}
}

//==============================================================

button_processor_t create_button_processor(){
	return new ButtonProcessor();
}

void set_on_click(button_processor_t proc, btn_func_t func){
	ButtonProcessor* p = static_cast<ButtonProcessor*>(proc);
	p->setOnClick(func);
}

void set_on_press_start(button_processor_t proc, btn_func_t func){
	ButtonProcessor* p = static_cast<ButtonProcessor*>(proc);
	p->setOnPressStart(func);
}

void set_on_press_end(button_processor_t proc, btn_func_t func){
	ButtonProcessor* p = static_cast<ButtonProcessor*>(proc);
	p->setOnPressEnd(func);
}

void set_during_press(button_processor_t proc, btn_func_t func){
	ButtonProcessor* p = static_cast<ButtonProcessor*>(proc);
	p->setDuringPress(func);
}
