#include "LEDStrip.h"
#include "utils/Debug.h"

LEDStrip::LEDStrip(CRGB* buffer, int count)
    : leds(buffer), numLEDs(count) {}

void LEDStrip::begin() {
    clear();
    show();
}

void LEDStrip::show() {
    FastLED.show();  // relies on global FastLED config
}

void LEDStrip::clear() {
    fill_solid(leds, numLEDs, CRGB::Black);
}

void LEDStrip::setPixel(int index, CRGB color) {
    if (index >= 0 && index < numLEDs) {
        leds[index] = color;
    }
}

int LEDStrip::getLength() const {
    return numLEDs;
}

CRGB* LEDStrip::getBuffer() {
    return leds;
}

void LEDStrip::setBrightness(int b) {
    brightness = b;
    FastLED.setBrightness(b);
}
