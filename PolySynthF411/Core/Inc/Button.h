/*
 * Button.h
 *
 *  Created on: Aug 21, 2024
 *      Author: hayden
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_
#include "MCP23S17.h"

#define BUTTON_CHECK_INTERVAL 40

typedef void (*btn_func_t)(uint8_t); // the main C-friendly function pointer typedef

#ifdef __cplusplus
#include <functional>

#define DEBOUNCE_MS 10.0f
#define CLICK_MS 100.0f
#define PRESS_MS 250.0f
#define IDLE_MS 1500.0f
#define PRESS_INTERVAL 300.0f

//enum for the button IDs
enum ButtonID {
	Alt,
	Env1,
	Env2,
	LFO1B,
	LFO2B,
	LFO3B,
	Osc1,
	Osc2,
	PWMB,
	FilterMode,
	FoldFirst,
	Menu,
	MS1,
	MS2,
	MS3,
	MS4,
	MS5,
	MS6,
	ModBank,
	MD1,
	MD2,
	MD3,
	MD4,
	MD5,
	MD6,
	MD7,
	MD8
};

#define NUM_BUTTONS 27

// button mapping to MCP23S17 wiring

// Expander 3
#define B_ALT_ADDR EXP_3_ADDR
#define B_ALT_PORT 0
#define B_ALT_PIN 0
#define B_ENV1_ADDR EXP_3_ADDR
#define B_ENV1_PORT 0
#define B_ENV1_PIN 1
#define B_ENV2_ADDR EXP_3_ADDR
#define B_ENV2_PORT 0
#define B_ENV2_PIN 2
#define B_LFO1_ADDR EXP_3_ADDR
#define B_LFO1_PORT 0
#define B_LFO1_PIN 3
#define B_LFO2_ADDR EXP_3_ADDR
#define B_LFO2_PORT 0
#define B_LFO2_PIN 4
#define B_LFO3_ADDR EXP_3_ADDR
#define B_LFO3_PORT 0
#define B_LFO3_PIN 5
#define B_OSC1_ADDR EXP_3_ADDR
#define B_OSC1_PORT 0
#define B_OSC1_PIN 6
#define B_OSC2_ADDR EXP_3_ADDR
#define B_OSC2_PORT 0
#define B_OSC2_PIN 7
// expander 2
#define B_MENU_ADDR EXP_2_ADDR
#define B_MENU_PORT 1
#define B_MENU_PIN 0
#define B_PWM_ADDR EXP_2_ADDR
#define B_PWM_PORT 1
#define B_PWM_PIN 5
#define B_FILTERMODE_ADDR EXP_2_ADDR
#define B_FILTERMODE_PORT 1
#define B_FILTERMODE_PIN 6
#define B_FOLDFIRST_ADDR EXP_2_ADDR
#define B_FOLDFIRST_PORT 1
#define B_FOLDFIRST_PIN 7
// expander 1
#define B_MS1_ADDR EXP_1_ADDR
#define B_MS1_PORT 0
#define B_MS1_PIN 0
#define B_MS2_ADDR EXP_1_ADDR
#define B_MS2_PORT 0
#define B_MS2_PIN 1
#define B_MS3_ADDR EXP_1_ADDR
#define B_MS3_PORT 0
#define B_MS3_PIN 2
#define B_MS4_ADDR EXP_1_ADDR
#define B_MS4_PORT 0
#define B_MS4_PIN 3
#define B_MS5_ADDR EXP_1_ADDR
#define B_MS5_PORT 0
#define B_MS5_PIN 4
#define B_MS6_ADDR EXP_1_ADDR
#define B_MS6_PORT 0
#define B_MS6_PIN 5
#define B_MODBANK_ADDR EXP_1_ADDR
#define B_MODBANK_PORT 0
#define B_MODBANK_PIN 6
#define B_MD1_ADDR EXP_1_ADDR
#define B_MD1_PORT 1
#define B_MD1_PIN 0
#define B_MD2_ADDR EXP_1_ADDR
#define B_MD2_PORT 1
#define B_MD2_PIN 1
#define B_MD3_ADDR EXP_1_ADDR
#define B_MD3_PORT 1
#define B_MD3_PIN 2
#define B_MD4_ADDR EXP_1_ADDR
#define B_MD4_PORT 1
#define B_MD4_PIN 3
#define B_MD5_ADDR EXP_1_ADDR
#define B_MD5_PORT 1
#define B_MD5_PIN 4
#define B_MD6_ADDR EXP_1_ADDR
#define B_MD6_PORT 1
#define B_MD6_PIN 5
#define B_MD7_ADDR EXP_1_ADDR
#define B_MD7_PORT 1
#define B_MD7_PIN 6
#define B_MD8_ADDR EXP_1_ADDR
#define B_MD8_PORT 1
#define B_MD8_PIN 7

typedef struct {
	uint8_t addr;
	uint8_t port;
	uint8_t pin;
} button_t;

typedef std::function<void()> BtnCallback;

//====================================================================

enum BtnState {
	INIT, DOWN, UP, COUNT, PRESS, PRESSEND
};

class Btn {
private:
	const bool activeLevel;
	BtnState state;
	bool debouncedLevel = !activeLevel;
	bool lastDebounceLevel = !activeLevel;

	tick_t lastDebounceTime = 0;
	tick_t now = 0;
	tick_t startTime = 0;
	tick_t lastDuringPressTime = 0;

	uint8_t numClicks = 0;
	uint8_t maxClicks = 1;
	// helpers
	void reset();
	bool debounce(bool input);
	void fsm(bool level);

	BtnCallback onClick;
	bool hasOnClick = false;
	BtnCallback onPressStart;
	bool hasOnPressStart = false;
	BtnCallback onPressEnd;
	bool hasOnPressEnd = false;
	BtnCallback duringPress;
	bool hasDuringPress = false;

public:
	Btn();
	void tick(bool level);
	// callback setters
	void setOnClick(BtnCallback func) {
		onClick = func;
		hasOnClick = true;
	}
	void setOnPressStart(BtnCallback func) {
		onPressStart = func;
		hasOnPressStart = true;
	}
	void setOnPressEnd(BtnCallback func) {
		onPressEnd = func;
		hasOnPressEnd = true;
	}
	void setDuringPress(BtnCallback func) {
		duringPress = func;
		hasDuringPress = true;
	}
};

class ButtonProcessor {
private:
	Btn buttons[NUM_BUTTONS];
	button_t locations[NUM_BUTTONS];
	// callback setters with c-friendly function pointers
	btn_func_t onClickHandler = nullptr;
	btn_func_t onPressStartHandler = nullptr;
	btn_func_t onPressEndHandler = nullptr;
	btn_func_t duringPressHandler = nullptr;

public:
	ButtonProcessor();
	void checkButtons(); // heavy lifting happens here

	// callback setters
	void setOnClick(btn_func_t oc) {
		onClickHandler = oc;
	}
	void setOnPressStart(btn_func_t oc) {
		onPressStartHandler = oc;
	}
	void setOnPressEnd(btn_func_t oc) {
		onPressEndHandler = oc;
	}
	void setDuringPress(btn_func_t oc) {
		duringPressHandler = oc;
	}

};

#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/
typedef void *button_processor_t;

EXTERNC button_processor_t create_button_processor();

EXTERNC void set_on_click(button_processor_t proc, btn_func_t func);
EXTERNC void set_on_press_start(button_processor_t proc, btn_func_t func);
EXTERNC void set_on_press_end(button_processor_t proc, btn_func_t func);
EXTERNC void set_during_press(button_processor_t proc, btn_func_t func);

// this goes in the callback
EXTERNC void check_buttons(button_processor_t proc);

#undef EXTERNC

#endif /* INC_BUTTON_H_ */
