#pragma once

#include <FastLED.h>
#include "../audio/AudioProcessor.h"

class BaseAnimation {
public:
    virtual ~BaseAnimation() = default;
    virtual const char* getName() const { return "BaseAnimation"; }
    
    virtual void begin(CRGB* leds, int numLeds) {
        this->leds = leds;
        this->numLeds = numLeds;
    }

    virtual void update(const AudioFeatures& features) = 0;
    virtual void updateWithFeatures(const AudioFeatures& features) { update(features); }

protected:
    CRGB* leds = nullptr;
    int numLeds = 0;
};
