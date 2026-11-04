/*
 * Perlin.cpp
 *
 *  Created on: Sep 1, 2024
 *      Author: hayden
 */

#include "Perlin.h"

//=====================================================
static inline int fFloor(float v){
	int i = static_cast<int>(v);
	return (v < i) ? (i - 1) : i;
}


// this permutation table gets used a lot. It just contains all values 0-255 in a random order.
static const uint8_t permTable[256] = {
    151, 160, 137, 91,  90,  215,  131, 13,  201, 95,  96,  53,  194, 233, 7,   82, 140, 36,  103,
    30,  69,  142, 8,   99,  117,  240, 21,  10,  23,  190, 6,   148, 247, 120, 234, 75,  0,   26,
    197, 62,  94,  252, 219, 203, 37, 35,  11,  32,  57,  177, 33,  88,  237, 149, 56,  87,  174,
    20,  125, 136, 171, 168, 68,  175, 74,  165, 71,  134, 139, 48,  27,  166, 19,  146, 158, 231,
    83,  111, 229, 122, 60,  211, 133, 230, 220, 105, 92,  41,  55,  46,  245, 40,  244, 102, 143,
    54,  65,  25,  63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,  18,  169, 200, 196,
    135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186, 3,   64,  52,  217, 226, 250, 124,
    123, 5,   202, 38,  147, 118, 126, 255, 225,  85,  212, 207, 206, 59,  227, 47,  16,  58,  17,
    182, 189, 28,  42,  223, 183, 170, 213, 119, 248, 152, 2,   44,  154, 163, 70,  221, 153, 101,
    155, 167, 43,  172, 9,   129, 22,  39,  253, 77,  98,  108, 110, 79,  113, 224, 232, 178, 185,
    112, 104, 218, 246, 97,  228, 251, 34,  242, 193, 238, 210, 144, 12,  191, 179, 162, 241, 81,
    51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157, 184, 84,  204, 176,
    115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205, 93,  222, 114, 67,  29,  24,  72,  243,
    141, 128, 195, 78,  66,  215, 61,  156, 180};

// hash function for a 32 bit int
static inline uint8_t hash32(int i){
	return permTable[static_cast<uint8_t>(i)];
}

// this guy calculates our gradient vectors
static float gradient(int num, float x){
	int hash = num & 0x0f;
	float grad = 1.0f + (hash & 7); // this value is in the range 1.0 <= grad < 8.0
	//randomly flip the sign sometimes
	if(hash & 0x07){
		grad *= -1.0f;
	}
	return grad * x;
}

// the main heavy lifting
float getPerlin1D(float x){
	float n0 = 0.0f;
	float n1 = 0.0f;
	int i0 = fFloor(x);
	int i1 = i0 + 1;

	float x0 = x - (float)i0;
	float x1 = x0 - 1.0f;

	// fisrt corner
	float t0 = 1.0f - (x0 * x0);
	t0 *= t0;
	n0 = t0 * t0 * gradient(hash32(i0), x0);

	// second corner
	float t1 = 1.0f * (x1 * x1);
	t1 *= t1;
	n1 = t1 * t1 * gradient(hash32(i1), x1);

	// add the corners
	float val = n0 + n1;
	// scale our output range from -2.532, 2.532 to 0, 1
	return ((val + 2.532f) / 2.532f);
}

// This adds together multiple octaves of noise from the above

float getPerlinFractal(float x, uint8_t octaves, float hz, float lac){
	float output = 0.0f;
	float denom = 0.0f;
	float amp = 1.0f;

	for(uint8_t o = 0; o < octaves; o++){
		output += (amp * getPerlin1D(x * hz));
		denom += amp;
		hz *= lac;
	}
	return output / denom;
}

//=====================================================
PerlinGen::PerlinGen(){

}


uint16_t PerlinGen::getNextValue(float hz){
		// check how much time has elapsed since last update
		tick_t now = TickTimer_get();
		float freqUpdate = 1000.0f / TickTimer_tickDistanceMs(lastUpdateTick, now);
		lastUpdateTick = now;
		// increment the x position
		xPos += (hz / (freqUpdate * 9.0f));
		return (uint16_t)getPerlinFractal(xPos, octaves, hz, lacunarity) * 4096.0f;
}
