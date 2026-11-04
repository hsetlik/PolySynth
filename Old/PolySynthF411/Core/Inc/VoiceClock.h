/*
 * VoiceClock.h
 *
 *  Created on: May 25, 2024
 *      Author: hayden
 */

#ifndef INC_VOICECLOCK_H_
#define INC_VOICECLOCK_H_
#include "main.h"
#include "StatusLED.h"
#define NUM_OSCS NUM_VOICES * 2
#define VOICE_CLOCK_BUF_SIZE 256


// protect the C++ stuff
#ifdef __cplusplus

typedef uint16_t voice_tick_t;
// helpers
namespace Timing {
float getActualUpdateHz(uint32_t masterClk, uint32_t updateHz);
float getTickHz(uint32_t clkFreq, uint8_t dataWidth);
}; // namespace Timing

class VoiceClock {
private:
  // helper for timing math
  float tickHz;

  // oscillator state stuff
#ifndef FLOAT_VOICE_MODE
  voice_tick_t fullTicks[NUM_OSCS];
  voice_tick_t halfTicks[NUM_OSCS];
  voice_tick_t currentTicks[NUM_OSCS];
#else
  float phaseDelta[NUM_OSCS];
  float phase[NUM_OSCS];

#endif
  uint16_t oscState;
  uint16_t dmaBuf[VOICE_CLOCK_BUF_SIZE];
  // the heavy lifting
  void tick();
  uint16_t getNext();
  // callback helpers
  void fillHalfBuffer(uint16_t* buf);
public:
  VoiceClock();
  void setFrequency(uint8_t osc, float hz);
  void begin(SPI_HandleTypeDef* spi); // start the circular DMA transmission
  void txHalfFinished();
  void txFinished();
};
#endif // __cplusplus

// wrapper we need to call this stuff from main.c
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef void *voice_clock_t;

EXTERNC voice_clock_t create_voice_clock();
EXTERNC void destroy_voice_clock(voice_clock_t clk);
EXTERNC void begin(voice_clock_t clk, SPI_HandleTypeDef* spi);
EXTERNC void tx_half_complete(voice_clock_t clk);
EXTERNC void tx_complete(voice_clock_t clk);
EXTERNC void set_osc_frequency(voice_clock_t clk, uint8_t osc, float hz);



#undef EXTERNC

#endif /* INC_VOICECLOCK_H_ */
