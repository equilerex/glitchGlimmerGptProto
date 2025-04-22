#pragma once

#include <FastLED.h>
#include "../audio/AudioProcessor.h"

class BaseAnimation {
public:
    virtual ~BaseAnimation() = default;

    virtual const char* getName() = 0;

    virtual void begin(CRGB* leds, int numLeds) {
        this->leds = leds;
        this->numLeds = numLeds;
    }

    virtual void update(const AudioFeatures& audio) = 0; // Simplify to use only AudioFeatures

protected:
    CRGB* leds = nullptr;
    int numLeds = 0;
};
