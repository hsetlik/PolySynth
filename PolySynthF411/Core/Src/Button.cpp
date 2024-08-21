/*
 * Button.cpp
 *
 *  Created on: Aug 21, 2024
 *      Author: hayden
 */
#include "Button.h"

// helpers to untangle the addr/port/pin business
button_t buttonForID(uint8_t idx){
	button_t b;
	ButtonID id = (ButtonID)idx;
	switch(id){
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
	case LFO1:
		b.addr = B_LFO1_ADDR;
		b.port = B_LFO1_PORT;
		b.pin = B_LFO1_PIN;
		break;
	case LFO2:
		b.addr = B_LFO2_ADDR;
		b.port = B_LFO2_PORT;
		b.pin = B_LFO2_PIN;
		break;
	case LFO3:
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
	case PWM:
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
		b.port = B_MD1_PORT;
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

uint8_t buttonForPin(uint8_t addr, uint8_t port, uint8_t pin){
	// 1. which expander are we on?
	switch(addr){
	case EXP_1_ADDR:
		if(port){ // MD buttons
			switch(pin){
			case B_MD1_PIN:
				return (uint8_t)ButtonID::MD1;
			case B_MD2_PIN:
				return (uint8_t)ButtonID::MD2;
			case B_MD3_PIN:
				return (uint8_t)ButtonID::MD3;
			case B_MD4_PIN:
				return (uint8_t)ButtonID::MD4;
			case B_MD5_PIN:
				return (uint8_t)ButtonID::MD5;
			case B_MD6_PIN:
				return (uint8_t)ButtonID::MD6;
			case B_MD7_PIN:
				return (uint8_t)ButtonID::MD7;
			case B_MD8_PIN:
				return (uint8_t)ButtonID::MD8;
			default:
				break;
			}
		} else { // MS and ModBank buttons
			switch(pin){
			case B_MS1_PIN:
				return (uint8_t)ButtonID::MS1;
			case B_MS2_PIN:
				return (uint8_t)ButtonID::MS2;
			case B_MS3_PIN:
				return (uint8_t)ButtonID::MS3;
			case B_MS4_PIN:
				return (uint8_t)ButtonID::MS4;
			case B_MS5_PIN:
				return (uint8_t)ButtonID::MS5;
			case B_MS6_PIN:
				return (uint8_t)ButtonID::MS6;
			case B_MODBANK_PIN:
				return (uint8_t)ButtonID::ModBank;
			}

		}
		break;
	case EXP_2_ADDR: // we don't need to check the port bc the only buttons are on port B
		switch(pin){
		case B_MENU_PIN:
			return (uint8_t)ButtonID::Menu;
		case B_PWM_PIN:
			return (uint8_t)ButtonID::PWM;
		case B_FILTERMODE_PIN:
			return (uint8_t)ButtonID::FilterMode;
		case B_FOLDFIRST_PIN:
			return (uint8_t)ButtonID::FoldFirst;
		default:
			break;
		}
		break;
	case EXP_3_ADDR: // again no need to check port bc all the buttons are on port B
		switch(pin){
		case B_ALT_PIN:
			return (uint8_t)ButtonID::Alt;
		case B_ENV1_PIN:
			return (uint8_t)ButtonID::Env1;
		case B_ENV2_PIN:
			return (uint8_t)ButtonID::Env2;
		case B_LFO1_PIN:
			return (uint8_t)ButtonID::LFO1;
		case B_LFO2_PIN:
			return (uint8_t)ButtonID::LFO2;
		case B_LFO3_PIN:
			return (uint8_t)ButtonID::LFO3;
		case B_OSC1_PIN:
			return (uint8_t)ButtonID::Osc1;
		case B_OSC2_PIN:
			return (uint8_t)ButtonID::Osc2;
		default:
			break;
		}
		break;
	}
	return 0;
}

//===================================================================




