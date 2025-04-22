#pragma once

#include <FastLED.h>
#include <vector>
#include <algorithm>
#include "../audio/AudioFeatures.h"
#include "pipeline/AnimationPipeline.h"

// ========== FirestormAnimation ==========
class FirestormAnimation : public BaseAnimation {
    CRGB* leds; int numLeds; uint8_t* heat;
public:
    static const char* staticName() { return "Firestorm"; }
    FirestormAnimation(CRGB* b, int n) : leds(b), numLeds(n), heat(new uint8_t[n]()) {}
    void begin() override { fill_solid(leds, numLeds, CRGB::Black); }
    void update(const AudioFeatures& f) override {
        for (int i = 0; i < numLeds; i++) heat[i] = qsub8(heat[i], random8(0, constrain((f.volume * 255) / 12, 2, 10)));
        for (int k = numLeds - 1; k >= 2; k--) heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
        if (f.beatDetected) for (int j = 0; j < 3; j++) heat[random8(numLeds / 4)] = qadd8(heat[random8(numLeds / 4)], random8(180, 255));
        for (int j = 0; j < numLeds; j++) leds[j] = HeatColor(heat[j]);
    }
    const char* getName() const override { return "Firestorm"; }
    ~FirestormAnimation() { delete[] heat; }
};

// ========== RippleCascadeAnimation ==========
class RippleCascadeAnimation : public BaseAnimation {
    CRGB* leds; int numLeds; uint8_t rippleColor = 0; int rippleStep = -1;
public:
    static const char* staticName() { return "Ripple Cascade"; }
    RippleCascadeAnimation(CRGB* b, int n) : leds(b), numLeds(n) {}
    void begin() override { fill_solid(leds, numLeds, CRGB::Black); rippleStep = -1; }
    void update(const AudioFeatures& f) override {
        if (f.beatDetected) { rippleColor = random8(); rippleStep = 0; }
        fadeToBlackBy(leds, numLeds, 64);
        if (rippleStep >= 0) {
            for (int i = 0; i < numLeds; i++) {
                int dist = abs((numLeds / 2) - i);
                if (dist == rippleStep) leds[i] = CHSV(rippleColor, 255, 255 - dist * 20);
            }
            rippleStep++;
            if (rippleStep > numLeds / 2) rippleStep = -1;
        }
    }
    const char* getName() const override { return "Ripple Cascade"; }
};

// ========== ColorTunnelAnimation ==========
class ColorTunnelAnimation : public BaseAnimation {
    CRGB* leds; int numLeds; uint8_t hue = 0;
public:
    static const char* staticName() { return "Color Tunnel"; }
    ColorTunnelAnimation(CRGB* b, int n) : leds(b), numLeds(n) {}
    void begin() override { fill_solid(leds, numLeds, CRGB::Black); }
    void update(const AudioFeatures& f) override {
        hue += f.volume * 8;
        for (int i = 0; i < numLeds; i++) leds[i] = CHSV(hue + i * 3, 255, sin8(i * 5 + millis() / 12));
        if (f.beatDetected) for (int i = 0; i < numLeds; i += 2) leds[i] += CHSV(hue + i * 3, 255, 255);
    }
    const char* getName() const override { return "Color Tunnel"; }
};

// ========== EnergySwirlAnimation ==========
class EnergySwirlAnimation : public BaseAnimation {
    CRGB* leds; int numLeds; uint8_t swirl = 0;
public:
    static const char* staticName() { return "Energy Swirl"; }
    EnergySwirlAnimation(CRGB* b, int n) : leds(b), numLeds(n) {}
    void begin() override { fill_solid(leds, numLeds, CRGB::Black); }
    void update(const AudioFeatures& f) override {
        swirl += f.mid * 8;
        for (int i = 0; i < numLeds; i++) leds[i] = CHSV(i * 5 + swirl, 255, f.volume * 255);
        blur1d(leds, numLeds, 30);
    }
    const char* getName() const override { return "Energy Swirl"; }
};

// ========== StrobeMatrixAnimation ==========
class StrobeMatrixAnimation : public BaseAnimation {
    CRGB* leds; int numLeds; bool state = false; unsigned long lastChange = 0;
public:
    static const char* staticName() { return "Strobe Matrix"; }
    StrobeMatrixAnimation(CRGB* b, int n) : leds(b), numLeds(n) {}
    void begin() override { fill_solid(leds, numLeds, CRGB::Black); lastChange = millis(); }
    void update(const AudioFeatures& f) override {
        if (millis() - lastChange > (f.bass > 0.5 ? 60 : 180)) {
            state = !state; lastChange = millis();
        }
        if (state) for (int i = 0; i < numLeds; i += random8(1, 5)) leds[i] = CHSV(random8(), 255, 255);
        else fill_solid(leds, numLeds, CRGB::Black);
    }
    const char* getName() const override { return "Strobe Matrix"; }
};

// ========== BassBloomAnimation ==========
class BassBloomAnimation : public BaseAnimation {
    CRGB* leds; int numLeds; uint8_t hue = 0; int size = 0;
public:
    static const char* staticName() { return "Bass Bloom"; }
    BassBloomAnimation(CRGB* b, int n) : leds(b), numLeds(n) {}
    void begin() override { fill_solid(leds, numLeds, CRGB::Black); size = 0; }
    void update(const AudioFeatures& f) override {
        if (f.bass > 0.5 || f.beatDetected) { size = numLeds / 2; hue = random8(); }
        fadeToBlackBy(leds, numLeds, 25);
        for (int i = 0; i < size; i++) {
            int l = (numLeds / 2) - i;
            int r = (numLeds / 2) + i;
            if (l >= 0) leds[l] += CHSV(hue + i * 2, 255, 255 - i * 5);
            if (r < numLeds) leds[r] += CHSV(hue + i * 2, 255, 255 - i * 5);
        }
        if (size > 0) size--;
    }
    const char* getName() const override { return "Bass Bloom"; }
};

// ========== ColorDripAnimation ==========
class ColorDripAnimation : public BaseAnimation {
    CRGB* leds; int numLeds; uint8_t hue = 0; std::vector<int> drips;
public:
    static const char* staticName() { return "Color Drip"; }
    ColorDripAnimation(CRGB* b, int n) : leds(b), numLeds(n) {}
    void begin() override { fill_solid(leds, numLeds, CRGB::Black); drips.clear(); }
    void update(const AudioFeatures& f) override {
        fadeToBlackBy(leds, numLeds, 30);
        if (f.treble > 0.25 || random8() < 4) { drips.push_back(0); hue += random8(5, 15); }
        for (int i = 0; i < drips.size(); ++i) {
            int pos = drips[i];
            if (pos < numLeds) {
                leds[pos] = CHSV(hue, 200, 255);
                if (pos > 0) leds[pos - 1].fadeToBlackBy(180);
                drips[i]++;
            }
        }
        drips.erase(std::remove_if(drips.begin(), drips.end(), [this](int p) { return p >= numLeds; }), drips.end());
    }
    const char* getName() const override { return "Color Drip"; }
};

// ========== FrequencyRiverAnimation ==========
class FrequencyRiverAnimation : public BaseAnimation {
    CRGB* leds; int numLeds;
public:
    static const char* staticName() { return "Frequency River"; }
    FrequencyRiverAnimation(CRGB* b, int n) : leds(b), numLeds(n) {}
    void begin() override { fill_solid(leds, numLeds, CRGB::Black); }
    void update(const AudioFeatures& f) override {
        int third = numLeds / 3;
        fill_solid(leds, third, CHSV(160, 255, f.bass * 255));
        fill_solid(leds + third, third, CHSV(96, 255, f.mid * 255));
        fill_solid(leds + 2 * third, third, CHSV(0, 255, f.treble * 255));
        blur1d(leds, numLeds, 16);
    }
    const char* getName() const override { return "Frequency River"; }
};

// ========== BioSignalAnimation ==========
class BioSignalAnimation : public BaseAnimation {
    CRGB* leds; int numLeds; uint8_t offset = 0;
public:
    static const char* staticName() { return "Bio Signal"; }
    BioSignalAnimation(CRGB* b, int n) : leds(b), numLeds(n) {}
    void begin() override { fill_solid(leds, numLeds, CRGB::Black); offset = 0; }
    void update(const AudioFeatures& f) override {
        offset += 2;
        uint8_t breath = sin8(millis() / 12);
        uint8_t brightness = (f.bass > 0.3) ? breath : 25;
        for (int i = 0; i < numLeds; i++) {
            uint8_t wave = sin8(i * 3 + offset);
            leds[i] = CHSV(wave + offset, 220, brightness);
            if (random8() < f.treble * 200) {
                leds[i] += CHSV(random8(), 255, 255);
            }
        }
        if (f.beatDetected) {
            for (int i = 0; i < numLeds; i++) {
                leds[i] += CHSV(0, 0, 40);
            }
        }
        blur1d(leds, numLeds, 30);
    }
    const char* getName() const override { return "Bio Signal"; }
};

// ========== ChaosEngineAnimation ==========
class ChaosEngineAnimation : public BaseAnimation {
    CRGB* leds; int numLeds;
public:
    static const char* staticName() { return "Chaos Engine"; }
    ChaosEngineAnimation(CRGB* b, int n) : leds(b), numLeds(n) {}
    void begin() override { fill_solid(leds, numLeds, CRGB::Black); }
    void update(const AudioFeatures&) override {
        fill_rainbow(leds, numLeds, millis() / 10, 7);
    }
    const char* getName() const override { return "Chaos Engine"; }
};

// ========== GalacticDriftAnimation ==========
class GalacticDriftAnimation : public BaseAnimation {
    CRGB* leds; int numLeds;
public:
    static const char* staticName() { return "Galactic Drift"; }
    GalacticDriftAnimation(CRGB* b, int n) : leds(b), numLeds(n) {}
    void begin() override { fill_solid(leds, numLeds, CRGB::Black); }
    void update(const AudioFeatures& f) override {
        for (int i = 0; i < numLeds; i++) {
            leds[i] = CHSV((i * 4 + millis() / 5) % 255, 255, sin8(i * 3 + millis() / 7));
        }
    }
    const char* getName() const override { return "Galactic Drift"; }
};

// ========== AudioStormAnimation ==========
class AudioStormAnimation : public BaseAnimation {
    CRGB* leds; int numLeds; uint8_t baseHue = 0;
public:
    static const char* staticName() { return "Audio Storm"; }
    AudioStormAnimation(CRGB* b, int n) : leds(b), numLeds(n) {}
    void begin() override { fill_solid(leds, numLeds, CRGB::Black); baseHue = 0; }
    void update(const AudioFeatures& f) override {
        baseHue += f.volume * 10;
        for (int i = 0; i < numLeds; i++) {
            leds[i] = CHSV(baseHue + i * 5, 255, f.beatDetected ? 255 : 128);
        }
        fadeToBlackBy(leds, numLeds, 10);
    }
    const char* getName() const override { return "Audio Storm"; }
};

// ========== SpectrumWavesAnimation ==========
class SpectrumWavesAnimation : public BaseAnimation {
    CRGB* leds; int numLeds;
public:
    static const char* staticName() { return "Spectrum Waves"; }
    SpectrumWavesAnimation(CRGB* b, int n) : leds(b), numLeds(n) {}
    void begin() override { fill_solid(leds, numLeds, CRGB::Black); }
    void update(const AudioFeatures& f) override {
        for (int i = 0; i < numLeds; i++) {
            float t = static_cast<float>(i) / numLeds;
            float wave = sinf(t * 10.0f + millis() * 0.005f);
            uint8_t level = static_cast<uint8_t>(constrain(f.bass * 255.0f * wave, 0, 255));
            leds[i] = CHSV(t * 255, 255, level);
        }
    }
    const char* getName() const override { return "Spectrum Waves"; }
};

