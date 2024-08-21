/*
 * Encoder.h
 *
 *  Created on: Aug 20, 2024
 *      Author: hayden
 */

#ifndef INC_ENCODER_H_
#define INC_ENCODER_H_
#include "MCP23S17.h"

#ifdef __cplusplus


typedef void (*enc_func_t)(uint8_t, uint8_t); // < (encoderID, clockwise)

#define NUM_ENCODERS 10

// an enum of hardware rotary encoders
enum EncID{
	A,
	B,
	C,
	D,
	Menu,
	Depth,
	Cutoff,
	Res,
	PWM,
	Fold
};
// the the address, port, and pins for all the encoders

// ADDRESSES --------------------
#define ENC_A_ADDR EXP_3_ADDR
#define ENC_B_ADDR EXP_3_ADDR
#define ENC_C_ADDR EXP_3_ADDR
#define ENC_D_ADDR EXP_3_ADDR

#define ENC_MENU_ADDR EXP_2_ADDR
#define ENC_DEPTH_ADDR EXP_2_ADDR
#define ENC_CUTOFF_ADDR EXP_2_ADDR
#define ENC_RES_ADDR EXP_2_ADDR
#define ENC_PWM_ADDR EXP_2_ADDR
#define ENC_FOLD_ADDR EXP_2_ADDR

// PORTS ------------------------
#define ENC_A_PORT 1
#define ENC_B_PORT 1
#define ENC_C_PORT 1
#define ENC_D_PORT 1

#define ENC_MENU_PORT 1
#define ENC_DEPTH_PORT 1
#define ENC_CUTOFF_PORT 0
#define ENC_RES_PORT 0
#define ENC_PWM_PORT 0
#define ENC_FOLD_PORT 0

// PINS --------------------------
#define ENC_A_PINL 7
#define ENC_A_PINR 6
#define ENC_B_PINL 5
#define ENC_B_PINR 4
#define ENC_C_PINL 3
#define ENC_C_PINR 2
#define ENC_D_PINL 1
#define ENC_D_PINR 0

#define ENC_MENU_PINL 2
#define ENC_MENU_PINR 1
#define ENC_DEPTH_PINL 3
#define ENC_DEPTH_PINR 4
#define ENC_CUTOFF_PINL 7
#define ENC_CUTOFF_PINR 6
#define ENC_RES_PINL 5
#define ENC_RES_PINR 4
#define ENC_PWM_PINL 3
#define ENC_PWM_PINR 2
#define ENC_FOLD_PINL 1
#define ENC_FOLD_PINR 0

typedef struct {
	uint8_t addr; // address of the MCP23S17
	uint8_t port;
	uint8_t pinL;
	uint8_t pinR;

} encoder_t;

//===================================================================
class EncoderProcessor
{
private:
	encoder_t encoders[NUM_ENCODERS];
	uint8_t isMoving[NUM_ENCODERS];
	enc_func_t callback;
public:
	EncoderProcessor();
	// set the port expander's registers up for handling interrupts on the encoder pins
	void initEncoderInterrupts();
	// call this from main.c in the relevant ISR
	void interruptSent(uint8_t addr, uint8_t port);
	// assign the function pointer
	void registerCallback(enc_func_t cb) {callback = cb;}
};



#endif

#ifdef __cplusplus
#define EXTERNC extern "c"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/
typedef void* enc_processor_t;

enc_processor_t create_enc_processor();
void enc_interrupt_sent(enc_processor_t proc, uint8_t addr, uint8_t port);
void enc_init_interrupts(enc_processor_t proc);
void enc_register_callback(enc_processor_t proc, enc_func_t func);


#undef EXTERNC

#endif /* INC_ENCODER_H_ */
