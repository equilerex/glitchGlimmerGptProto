#pragma once

#include <FastLED.h>
#include "../config/Config.h"

#ifndef LED_STRIP_COUNT_1
#define LED_STRIP_COUNT_1 60
#endif
#ifndef LED_STRIP_PIN_1
#define LED_STRIP_PIN_1 5
#endif
#ifndef LED_STRIP_COUNT_2
#define LED_STRIP_COUNT_2 60
#endif
#ifndef LED_STRIP_PIN_2
#define LED_STRIP_PIN_2 18
#endif

extern CRGB leds1[LED_STRIP_COUNT_1];
extern CRGB leds2[LED_STRIP_COUNT_2];

inline void setupLEDs() {
    
    Debug::logf(Debug::INFO, "-------------------------- LED %p, numLEDs=%d", LED_STRIP_PIN_1, LED_STRIP_COUNT_1);
    FastLED.addLeds<WS2811, LED_STRIP_PIN_1, GRB>(leds1, LED_STRIP_COUNT_1);
    FastLED.addLeds<WS2811, LED_STRIP_PIN_2, GRB>(leds2, LED_STRIP_COUNT_2);
    
    Debug::logf(Debug::INFO, "***************************** LED %p, numLEDs=%d", leds1, leds2);

    FastLED.show();
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
} 
