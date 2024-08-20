/*
 * Encoder.cpp
 *
 *  Created on: Aug 20, 2024
 *      Author: hayden
 */
#include "Encoder.h"

encoder_t encoderForID(uint8_t idx) {
	encoder_t enc;
	EncID id = (EncID) idx;
	switch (id) {
	case A:
		enc.addr = ENC_A_ADDR;
		enc.port = ENC_A_PORT;
		enc.pinL = ENC_A_PINL;
		enc.pinR = ENC_A_PINR;
		break;
	case B:
		enc.addr = ENC_B_ADDR;
		enc.port = ENC_B_PORT;
		enc.pinL = ENC_B_PINL;
		enc.pinR = ENC_B_PINR;
		break;
	case C:
		enc.addr = ENC_C_ADDR;
		enc.port = ENC_C_PORT;
		enc.pinL = ENC_C_PINL;
		enc.pinR = ENC_C_PINR;
		break;
	case D:
		enc.addr = ENC_D_ADDR;
		enc.port = ENC_D_PORT;
		enc.pinL = ENC_D_PINL;
		enc.pinR = ENC_D_PINR;
		break;
	case Cutoff:
		enc.addr = ENC_CUTOFF_ADDR;
		enc.port = ENC_CUTOFF_PORT;
		enc.pinL = ENC_CUTOFF_PINL;
		enc.pinR = ENC_CUTOFF_PINR;
		break;
	case Res:
		enc.addr = ENC_RES_ADDR;
		enc.port = ENC_RES_PORT;
		enc.pinL = ENC_RES_PINL;
		enc.pinR = ENC_RES_PINR;
		break;
	case Menu:
		enc.addr = ENC_MENU_ADDR;
		enc.port = ENC_MENU_PORT;
		enc.pinL = ENC_MENU_PINL;
		enc.pinR = ENC_MENU_PINR;
		break;
	case Depth:
		enc.addr = ENC_DEPTH_ADDR;
		enc.port = ENC_DEPTH_PORT;
		enc.pinL = ENC_DEPTH_PINL;
		enc.pinR = ENC_DEPTH_PINR;
		break;
	case PWM:
		enc.addr = ENC_PWM_ADDR;
		enc.port = ENC_PWM_PORT;
		enc.pinL = ENC_PWM_PINL;
		enc.pinR = ENC_PWM_PINR;
		break;
	case Fold:
		enc.addr = ENC_FOLD_ADDR;
		enc.port = ENC_FOLD_PORT;
		enc.pinL = ENC_FOLD_PINL;
		enc.pinR = ENC_FOLD_PINR;
		break;
	default:
		break;
	}
	return enc;
}

uint8_t encoderForPin(uint8_t addr, uint8_t port, uint8_t pin) {
	switch (addr) {
	case EXP_2_ADDR:
		if(port) {
			switch(pin){
			case ENC_MENU_PINR:
				return (uint8_t)EncID::Menu;
			case ENC_MENU_PINL:
				return (uint8_t)EncID::Menu;
			case ENC_DEPTH_PINR:
				return (uint8_t)EncID::Depth;
			case ENC_DEPTH_PINL:
				return (uint8_t)EncID::Depth;
			}

		} else {
			switch(pin){
			case ENC_FOLD_PINR:
				return (uint8_t)EncID::Fold;
			case ENC_FOLD_PINL:
				return (uint8_t)EncID::Fold;
			case ENC_PWM_PINR:
				return (uint8_t)EncID::PWM;
			case ENC_PWM_PINL:
				return (uint8_t)EncID::PWM;
			case ENC_RES_PINR:
				return (uint8_t)EncID::Res;
			case ENC_RES_PINL:
				return (uint8_t)EncID::Res;
			case ENC_CUTOFF_PINR:
				return (uint8_t)EncID::Cutoff;
			case ENC_CUTOFF_PINL:
				return (uint8_t)EncID::Cutoff;
			}
		}
		break;
	case EXP_3_ADDR:
		switch(pin){
		case ENC_A_PINR:
			return (uint8_t)EncID::A;
		case ENC_A_PINL:
			return (uint8_t)EncID::A;
		case ENC_B_PINR:
			return (uint8_t)EncID::B;
		case ENC_B_PINL:
			return (uint8_t)EncID::B;
		case ENC_C_PINR:
			return (uint8_t)EncID::C;
		case ENC_C_PINL:
			return (uint8_t)EncID::C;
		case ENC_D_PINR:
			return (uint8_t)EncID::D;
		case ENC_D_PINL:
			return (uint8_t)EncID::D;
		}
		break;
	}
	return 0;
}

//--------------------------------------------------------------------
EncoderProcessor::EncoderProcessor() {
	for (uint8_t e = 0; e < NUM_ENCODERS; e++) {
		// grip the data for each encoder
		encoders[e] = encoderForID(e);

	}
}

void EncoderProcessor::initEncoderInterrupts() {
	for (uint8_t enc = 0; enc < NUM_ENCODERS; enc++) {
		MCP23S17_setInterruptOnPin(&hspi2, encoders[enc].addr,
				encoders[enc].port, encoders[enc].pinL, 1);
		MCP23S17_setInterruptOnPin(&hspi2, encoders[enc].addr,
				encoders[enc].port, encoders[enc].pinR, 1);
	}
}

void EncoderProcessor::interruptSent(uint8_t addr, uint8_t port) {
	uint8_t pin = MCP23S17_getLastInterruptPin(&hspi2, addr, port);
	uint8_t encIdx = encoderForPin(addr, port, pin);
	if(!isMoving[encIdx]){ // the turn is starting
		isMoving[encIdx] = 1;
	} else { // the turn is ending
		//TODO: run the function pointer callback here
	}

}

