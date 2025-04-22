#pragma once
#include <FastLED.h>
#include "../config/Config.h"
#include "../core/Debug.h"

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
    
    // Helper method to verify buffer integrity
    bool checkBufferValid() const {
        if (!leds) {
            Debug::log(Debug::ERROR, "LED buffer is null");
            return false;
        }
        
        if (numLEDs <= 0) {
            Debug::logf(Debug::ERROR, "Invalid LED count: %d", numLEDs);
            return false;
        }
        
        return true;
    }
};
