#include "led_controller.h"

Adafruit_NeoPixel led_strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

const uint32_t activeColor = led_strip.Color(255, 255, 255); // White
const uint32_t inactiveColor = led_strip.Color(0, 0, 0);     // Off

// this will run constantly when a user is not in a game
void idleAnimation() {
  static uint16_t frame = 0;  // keeps track of animation step

  // Cycle brightness like a sine wave
  float brightnessFactor = (sin(frame * 0.05) + 1.0) / 2.0; // 0 → 1
  uint8_t level = (uint8_t)(brightnessFactor * 100);        // brightness (0-100%)

  for (int i = 0; i < LED_COUNT; i++) {
    // Each LED has a phase offset for a wave effect
    float wave = (sin((frame * 0.05) + (i * 0.3)) + 1.0) / 2.0;
    uint8_t waveLevel = (uint8_t)(wave * 255);

    // Subtle blue-white glow
    uint32_t color = led_strip.Color(waveLevel / 3, waveLevel / 3, waveLevel);
    led_strip.setPixelColor(i, color);
  }

  led_strip.setBrightness(level); // breathing brightness
  led_strip.show();

  frame++;
  delay(30);
}


void updateLEDs(uint64_t boardState) {
  static uint64_t lastState = 0;
  if (boardState == lastState) return; // No change

  lastState = boardState;

  for (int i = 0; i < LED_COUNT; i++) {
    if ((boardState >> i) & 1) {
      led_strip.setPixelColor(i, activeColor);
    } else {
      led_strip.setPixelColor(i, inactiveColor);
    }
  }

  led_strip.show();  // Apply changes
}

void clearAllLEDs() {
  for (int i = 0; i < LED_COUNT; i++) {
    led_strip.setPixelColor(i, inactiveColor);
  }
  led_strip.show();
}


void led_setup() {
    led_strip.begin();
    led_strip.setBrightness(100);
    led_strip.show(); // Start with all LEDs off
    clearAllLEDs();
}