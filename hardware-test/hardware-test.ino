/// @file    hardware-test.ino
/// @brief   Hardware test firmware (WS2815 chain, SP803E controller)
/// @see     docs/ELECTRONICS_GUIDE.md section 6

#include <FastLED.h>

// Gamma lookup table (copied from andetag-nafas helpers.h)
const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

// Set to the number of pixels in the data path under test (see guide, Test 6).
#define NUM_LEDS 666
#define DATA_PIN 16  // SP803E data pin (guide section 2)

// Frame cap, matches andetag-nafas production firmware
#define TARGET_FPS 60
#define FRAME_TIME (1000 / TARGET_FPS)

// One full wave traversal across the strip
#define WAVE_CYCLE_MS 4500

// cos8 argument step per pixel (256 = one spatial wavelength)
#define WAVE_SPATIAL 12

CRGB leds[NUM_LEDS];
unsigned long lastMillis = 0;

void setup() {
	delay(2000);
	FastLED.addLeds<WS2815, DATA_PIN, GRB>(leds, NUM_LEDS);
}

void loop() {
	if (millis() - lastMillis < FRAME_TIME) {
		return;
	}
	lastMillis = millis();

	uint16_t travel = ((millis() % WAVE_CYCLE_MS) * 256) / WAVE_CYCLE_MS;

	for (uint16_t i = 0; i < NUM_LEDS; i++) {
		uint8_t wave = 255 - cos8(i * WAVE_SPATIAL + travel);
		uint8_t brightness = pgm_read_byte(&gamma8[wave]);
		leds[i] = CRGB(brightness, brightness, brightness);
	}

	FastLED.show();
}
