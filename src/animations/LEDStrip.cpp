#include "LEDStrip.h"
#include "../core/Debug.h"

LEDStrip::LEDStrip(CRGB* buffer, int count)
    : leds(buffer), numLEDs(count) {
    Debug::logf(Debug::INFO, "LEDStrip constructor: buffer=0x%p, count=%d", buffer, count);
    if (!buffer) {
        Debug::log(Debug::ERROR, "LEDStrip created with null buffer!");
    }
    if (count <= 0) {
        Debug::logf(Debug::ERROR, "LEDStrip created with invalid count: %d", count);
    }
}

void LEDStrip::begin() {
    if (!leds || numLEDs <= 0) {
        Debug::log(Debug::ERROR, "Invalid LED strip configuration");
        return;
    }

    Debug::logf(Debug::INFO, "LEDStrip begin: buffer=0x%p, numLEDs=%d", leds, numLEDs);
    clear();
    show();
    Debug::logf(Debug::DEBUG, "LED strip initialized with %d LEDs", numLEDs);
}

void LEDStrip::show() {
    if (!leds) {
        Debug::log(Debug::ERROR, "Attempt to show null LED buffer");
        return;
    }

    // Extra check for potential memory issues
    static unsigned long lastWarningTime = 0;
    static int warningCounter = 0;

    if (numLEDs <= 0) {
        if (millis() - lastWarningTime > 5000) { // Limit warning frequency
            Debug::logf(Debug::ERROR, "Invalid LEDs count in show(): %d", numLEDs);
            lastWarningTime = millis();
            warningCounter++;
        }
        return;
    }

    // Debug: Log first few LEDs to avoid overwhelming serial output
    if (Debug::getLogLevel() >= Debug::DEBUG) {
        Debug::log(Debug::DEBUG, "LED buffer preview before update:");
        const int maxPreviewLEDs = min(5, numLEDs); // Preview at most 5 LEDs

        for (int i = 0; i < maxPreviewLEDs; i++) {
            // Safer access with bounds check
            if (i >= 0 && i < numLEDs) {
                Debug::logf(Debug::DEBUG, "  LED[%d] = R:%d G:%d B:%d",
                    i, leds[i].r, leds[i].g, leds[i].b);
            }
        }
    }

    // Performance timing for FastLED update
    unsigned long startTime = micros();

    try {
        FastLED.show();  // relies on global FastLED config
    } catch (const std::exception& e) {
        Debug::logf(Debug::ERROR, "Exception in FastLED.show(): %s", e.what());
    } catch (...) {
        Debug::log(Debug::ERROR, "Unknown exception in FastLED.show()");
    }

    unsigned long duration = micros() - startTime;

    if (duration > 10000) { // Log if update takes more than 10ms
        Debug::logf(Debug::INFO, "LED update took %lu microseconds", duration);
    }
}

void LEDStrip::clear() {
    if (!leds) {
        Debug::log(Debug::ERROR, "Attempt to clear null LED buffer");
        return;
    }

    if (numLEDs <= 0) {
        Debug::logf(Debug::ERROR, "Invalid LEDs count in clear(): %d", numLEDs);
        return;
    }

    try {
        fill_solid(leds, numLEDs, CRGB::Black);
    } catch (const std::exception& e) {
        Debug::logf(Debug::ERROR, "Exception in clear(): %s", e.what());
    } catch (...) {
        Debug::log(Debug::ERROR, "Unknown exception in clear()");
    }
}

void LEDStrip::setPixel(int index, CRGB color) {
    if (!leds) {
        Debug::log(Debug::ERROR, "Attempt to set pixel on null LED buffer");
        return;
    }

    if (index >= 0 && index < numLEDs) {
        try {
            leds[index] = color;
        } catch (...) {
            Debug::logf(Debug::ERROR, "Exception accessing LED[%d] in setPixel()", index);
        }
    } else {
        // Only log out-of-bounds errors occasionally to avoid flooding serial
        static unsigned long lastErrorTime = 0;
        if (millis() - lastErrorTime > 2000) { // Only log every 2 seconds at most
            Debug::logf(Debug::ERROR, "Invalid LED index: %d (max: %d)", index, numLEDs - 1);
            lastErrorTime = millis();
        }
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

    try {
        FastLED.setBrightness(b);
    } catch (...) {
        Debug::logf(Debug::ERROR, "Exception in FastLED.setBrightness(%d)", b);
    }

    Debug::logf(Debug::DEBUG, "LED brightness set to %d", b);
}
