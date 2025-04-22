#pragma once
#include <FastLED.h>
#include "../config/Config.h"

#pragma once
#include <FastLED.h>

class LEDStrip {
public:
    LEDStrip(CRGB* buffer, int count);

    void begin();
    void show();
    void clear();
    void setPixel(int index, CRGB color);
    int getLength() const;
    CRGB* getBuffer();
    void setBrightness(int b);

private:
    CRGB* leds = nullptr;
    int numLEDs = 0;
    int brightness = 128;
};
