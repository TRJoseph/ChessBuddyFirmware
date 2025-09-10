#ifndef LEDCONTROL
#define LEDCONTROL

#include <Adafruit_NeoPixel.h>

#define LED_PIN    18
#define LED_COUNT  64


/* FUNCTION DECLARATIONS */

void updateLEDs(uint64_t boardState);
void clearAllLEDs();
void led_setup();
void idleAnimation();

#endif