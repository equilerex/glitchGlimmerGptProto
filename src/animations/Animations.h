#pragma once

#include <vector>
#include <algorithm>
#include <FastLED.h>
#include "../audio/AudioProcessor.h"
#include "pipeline/AnimationPipeline.h"

class FirestormAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    uint8_t* heat = nullptr;

public:
    static const char* staticName() {
        return "Firestorm";
    }

    FirestormAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
        heat = new uint8_t[numLeds]();
    }

    void update(const AudioFeatures& features) override {
        for (int i = 0; i < numLeds; i++) {
            heat[i] = qsub8(heat[i], random8(0, constrain((features.volume * 255) / 12, 2, 10)));
        }

        for (int k = numLeds - 1; k >= 2; k--) {
            heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
        }

        if (features.beatDetected) {
            for (int j = 0; j < 3; j++) {
                heat[random8(numLeds / 4)] = qadd8(heat[random8(numLeds / 4)], random8(180, 255));
            }
        }

        for (int j = 0; j < numLeds; j++) {
            leds[j] = HeatColor(heat[j]);
        }
    }

    const char* getName() const override {
        return "Firestorm";
    }

    ~FirestormAnimation() {
        delete[] heat;
    }
};

class RippleCascadeAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    uint8_t rippleColor = 0;
    int rippleStep = -1;

public:
    static const char* staticName() {
        return "Ripple Cascade";
    }

    RippleCascadeAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
        rippleStep = -1;
    }
    
    void update(const AudioFeatures& features) override {
        if (features.beatDetected) {
            rippleColor = random8();
            rippleStep = 0;
        }

        fadeToBlackBy(leds, numLeds, 64);

        if (rippleStep >= 0) {
            for (int i = 0; i < numLeds; i++) {
                int dist = abs((numLeds / 2) - i);
                if (dist == rippleStep) {
                    leds[i] = CHSV(rippleColor, 255, 255 - dist * 20);
                }
            }
            rippleStep++;
            if (rippleStep > numLeds / 2) rippleStep = -1;
        }
    }

    const char* getName() const override {
        return "Ripple Cascade";
    }
};

class ColorTunnelAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    uint8_t hue = 0;

public:
    static const char* staticName() {
        return "Color Tunnel";
    }

    ColorTunnelAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
    }
    
    void update(const AudioFeatures& features) override {
        hue += features.volume * 8;

        for (int i = 0; i < numLeds; i++) {
            leds[i] = CHSV(hue + i * 3, 255, sin8(i * 5 + millis() / 12));
        }

        if (features.beatDetected) {
            for (int i = 0; i < numLeds; i += 2) {
                leds[i] += CHSV(hue + i * 3, 255, 255);
            }
        }
    }

    const char* getName() const override {
        return "Color Tunnel";
    }
};

class EnergySwirlAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    uint8_t swirl = 0;

public:
    static const char* staticName() {
        return "Energy Swirl";
    }

    EnergySwirlAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
    }
    
    void update(const AudioFeatures& features) override {
        swirl += features.mid * 8;

        for (int i = 0; i < numLeds; i++) {
            leds[i] = CHSV((i * 5 + swirl), 255, features.volume * 255);
        }

        blur1d(leds, numLeds, 30);
    }

    const char* getName() const override {
        return "Energy Swirl";
    }
};

class StrobeMatrixAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    bool state = false;
    unsigned long lastChange = 0;

public:
    static const char* staticName() {
        return "Strobe Matrix";
    }

    StrobeMatrixAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
        lastChange = millis();
    }
    
    void update(const AudioFeatures& features) override {
        if (millis() - lastChange > (features.bass > 0.5 ? 60 : 180)) {
            state = !state;
            lastChange = millis();
        }

        if (state) {
            for (int i = 0; i < numLeds; i += random8(1, 5)) {
                leds[i] = CHSV(random8(), 255, 255);
            }
        } else {
            fill_solid(leds, numLeds, CRGB::Black);
        }
    }

    const char* getName() const override {
        return "Strobe Matrix";
    }
};

class BassBloomAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    uint8_t hue = 0;
    int size = 0;

public:
    static const char* staticName() {
        return "Bass Bloom";
    }

    BassBloomAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
        size = 0;
    }
    
    void update(const AudioFeatures& features) override {
        if (features.bass > 0.5 || features.beatDetected) {
            size = numLeds / 2;
            hue = random8();
        }

        fadeToBlackBy(leds, numLeds, 25);

        for (int i = 0; i < size; i++) {
            int l = (numLeds / 2) - i;
            int r = (numLeds / 2) + i;
            if (l >= 0) leds[l] += CHSV(hue + i * 2, 255, 255 - i * 5);
            if (r < numLeds) leds[r] += CHSV(hue + i * 2, 255, 255 - i * 5);
        }

        if (size > 0) size--;
    }

    const char* getName() const override {
        return "Bass Bloom";
    }
};

class ColorDripAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    uint8_t hue = 0;
    std::vector<int> drips;

public:
    static const char* staticName() {
        return "Color Drip";
    }

    ColorDripAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
        drips.clear();
    }
    
    void update(const AudioFeatures& features) override {
        fadeToBlackBy(leds, numLeds, 30);

        if (features.treble > 0.25 || random8() < 4) {
            drips.push_back(0);
            hue += random8(5, 15);
        }

        for (int i = 0; i < drips.size(); ++i) {
            int pos = drips[i];
            if (pos < numLeds) {
                leds[pos] = CHSV(hue, 200, 255);
                if (pos > 0) leds[pos - 1].fadeToBlackBy(180);
                drips[i]++;
            }
        }

        drips.erase(std::remove_if(drips.begin(), drips.end(), [this](int p) {
            return p >= numLeds;
        }), drips.end());
    }

    const char* getName() const override {
        return "Color Drip";
    }
};

class FrequencyRiverAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;

public:
    static const char* staticName() {
        return "Frequency River";
    }

    FrequencyRiverAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
    }
    
    void update(const AudioFeatures& features) override {
        int third = numLeds / 3;
        fill_solid(leds, third, CHSV(160, 255, features.bass * 255));
        fill_solid(leds + third, third, CHSV(96, 255, features.mid * 255));
        fill_solid(leds + 2 * third, third, CHSV(0, 255, features.treble * 255));
        blur1d(leds, numLeds, 16);
    }

    const char* getName() const override {
        return "Frequency River";
    }
};

class PartyPulseAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    uint8_t hue = 0;
    int radius = 0;

public:
    static const char* staticName() {
        return "Party Pulse";
    }

    PartyPulseAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
        radius = 0;
    }
    
    void update(const AudioFeatures& features) override {
        if (features.beatDetected) hue += 30;

        fill_gradient(leds, numLeds, CHSV(hue, 255, features.volume * 180), CHSV(hue + 64, 255, features.volume * 180));

        if (features.bass > 0.5) radius = numLeds / 2;

        for (int i = 0; i < radius; i++) {
            int l = (numLeds / 2) - i;
            int r = (numLeds / 2) + i;
            if (l >= 0) leds[l] += CHSV(hue + 60, 255, 255 - i * 4);
            if (r < numLeds) leds[r] += CHSV(hue + 60, 255, 255 - i * 4);
        }
        if (radius > 0) radius--;

        for (int i = 0; i < numLeds / 6; i++) {
            if (random8() < features.treble * 255 || random8() < features.mid * 100) {
                leds[random16(numLeds)] += CHSV(hue + random8(), 200, 255);
            }
        }
        blur1d(leds, numLeds, 18);
    }

    const char* getName() const override {
        return "Party Pulse";
    }
};

class CyberFluxAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    uint8_t hue = 0;

public:
    static const char* staticName() {
        return "Cyber Flux";
    }

    CyberFluxAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
    }
    
    void update(const AudioFeatures& features) override {
        hue += features.volume * 4;

        if (features.bass > 0.4) {
            int center = random16(numLeds);
            leds[center] = CHSV(hue, 255, 255);
            if (center > 0) leds[center - 1] = CHSV(hue + 20, 255, 180);
            if (center < numLeds - 1) leds[center + 1] = CHSV(hue - 20, 255, 180);
        }

        for (int i = 0; i < numLeds; i++) {
            leds[i] += CHSV(hue + (i * 2), 255, sin8(i * 4 + millis() / 6));
        }

        for (int i = 0; i < numLeds; i++) {
            if (random8() < features.treble * 220) {
                leds[i] = CRGB::White;
            }
        }
        fadeToBlackBy(leds, numLeds, 22);
    }

    const char* getName() const override {
        return "Cyber Flux";
    }
};

class BioSignalAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    uint8_t offset = 0;

public:
    static const char* staticName() {
        return "Bio Signal";
    }

    BioSignalAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
        offset = 0;
    }
    
    void update(const AudioFeatures& features) override {
        offset += 2;

        uint8_t breath = sin8(millis() / 12);
        uint8_t brightness = (features.bass > 0.3) ? breath : 25;

        for (int i = 0; i < numLeds; i++) {
            uint8_t wave = sin8(i * 3 + offset);
            leds[i] = CHSV(wave + offset, 220, brightness);
            if (random8() < features.treble * 200) {
                leds[i] += CHSV(random8(), 255, 255);
            }
        }
        if (features.beatDetected) {
            for (int i = 0; i < numLeds; i++) {
                leds[i] += CHSV(0, 0, 40);
            }
        }
        blur1d(leds, numLeds, 30);
    }

    const char* getName() const override {
        return "Bio Signal";
    }
    };

    class ChaosEngineAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    
public:
    static const char* staticName() {
        return "Chaos Engine";
    }

    ChaosEngineAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
    }
    
    void update(const AudioFeatures&) override {
        fill_rainbow(leds, numLeds, millis() / 10, 7);
    }

    const char* getName() const override {
        return "Chaos Engine";
    }
};

class GalacticDriftAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    
public:
    static const char* staticName() {
        return "Galactic Drift";
    }

    GalacticDriftAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
    }
    
    void update(const AudioFeatures& features) override {
        for (int i = 0; i < numLeds; i++) {
            leds[i] = CHSV((i * 4 + millis() / 5) % 255, 255, sin8(i * 3 + millis() / 7));
        }
    }

    const char* getName() const override {
        return "Galactic Drift";
    }
};

class AudioStormAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    uint8_t baseHue = 0;

public:
    static const char* staticName() {
        return "Audio Storm";
    }

    AudioStormAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
        baseHue = 0;
    }
    
    void update(const AudioFeatures& features) override {
        baseHue += features.volume * 10;
        for (int i = 0; i < numLeds; i++) {
            leds[i] = CHSV(baseHue + i * 5, 255, features.beatDetected ? 255 : 128);
        }
        fadeToBlackBy(leds, numLeds, 10);
    }

    const char* getName() const override {
        return "Audio Storm";
    }
};

class SpectrumWavesAnimation : public BaseAnimation {
private:
    CRGB* leds = nullptr;
    int numLeds = 0;
    
public:
    static const char* staticName() {
        return "Spectrum Waves";
    }

    SpectrumWavesAnimation(CRGB* buffer, int count) : leds(buffer), numLeds(count) {
        // Constructor
    }
    
    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
    }
    
    void update(const AudioFeatures& audio) override {
        for (int i = 0; i < numLeds; i++) {
            float t = static_cast<float>(i) / numLeds;
            float wave = sinf(t * 10.0f + millis() * 0.005f);
            uint8_t level = static_cast<uint8_t>(constrain(audio.bass * 255.0f * wave, 0, 255));

            leds[i] = CHSV(t * 255, 255, level);
        }
    }

    const char* getName() const override {
        return "Spectrum Waves";
    }
};
