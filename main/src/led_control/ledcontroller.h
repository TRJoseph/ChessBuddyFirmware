#ifndef LEDCONTROL
#define LEDCONTROL

#include <Adafruit_NeoPixel.h>

#define LED_PIN    18
#define LED_COUNT  64

class LED_Controller {
private:
    Adafruit_NeoPixel led_strip;
    uint32_t activeColor;
    uint32_t inactiveColor;
public:
    LED_Controller();
    
    /* FUNCTION DECLARATIONS */

    void updateLEDs(uint64_t boardState);
    void clearAllLEDs();
    void led_setup();
    void idleAnimation();
};

#endif