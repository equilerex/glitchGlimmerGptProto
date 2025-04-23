#pragma once

#include <FastLED.h>
#include <deque>
#include "../audio/AudioFeatures.h"
#include "../audio/AudioSnapshot.h"

// AudioSnapshot is already defined in AudioSnapshot.h, no need for type alias

class VisualLayer {
public:
    virtual ~VisualLayer() = default;

    float opacity = 1.0f;
    String name = "Unnamed";

    // Optional shared state
    uint8_t hue = 0;
    float pulse = 0.0f;
    float squirtTimer = 0.0f;
    float trailOffset = 0.0f;

    virtual void update(const AudioFeatures& now, const std::deque<AudioSnapshot>& history) = 0;
    virtual void render(CRGB* leds, int count) = 0;

    // Add this method for logging
    virtual const char* getName() const {
        return "VisualLayer";
    }
};
