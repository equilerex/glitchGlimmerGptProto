#pragma once

#include "../audio/AudioFeatures.h"
#include "../animations/Animations.h"
#include <vector>
#include <FastLED.h>

class BaseAnimation {
public:
    virtual ~BaseAnimation() = default;
    virtual void begin() = 0;
    virtual void update(const AudioFeatures& features) = 0;
    virtual const char* getName() const = 0;
};

using AnimationFactory = BaseAnimation* (*)(CRGB* leds, int numLeds);

class AnimationManager {
public:
    AnimationManager() = default;

    void setup(CRGB* ledBuffer, int ledCount) {
        leds = ledBuffer;
        this->ledCount = ledCount;
    }

    void initialize(const std::vector<AnimationFactory>& registry) {
        for (auto& factory : registry) {
            animations.push_back(factory(leds, ledCount));
            animations.back()->begin();
        }
    }

    void update(const AudioFeatures& features) {
        if (!animations.empty()) {
            animations[currentIndex]->update(features);
        }
    }

    void next() {
        if (!animations.empty()) {
            currentIndex = (currentIndex + 1) % animations.size();
        }
    }

    void previous() {
        if (!animations.empty()) {
            currentIndex = (currentIndex - 1 + animations.size()) % animations.size();
        }
    }

    int getCurrentIndex() const { return currentIndex; }
    int getAnimationCount() const { return animations.size(); }
    const String getCurrentName() const {
        return animations.empty() ? "None" : animations[currentIndex]->getName();
    }
    BaseAnimation& getCurrent() { return *animations[currentIndex]; }

private:
    std::vector<BaseAnimation*> animations;
    CRGB* leds = nullptr;
    int ledCount = 0;
    int currentIndex = 0;
};

// --- Animations ---

class WaveformBounce : public BaseAnimation {
public:
    WaveformBounce(CRGB* buffer, int count) : leds(buffer), numLeds(count) {}

    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
    }

    void update(const AudioFeatures& features) override {
        fill_solid(leds, numLeds, CRGB::Black);
        if (!features.waveform) return;
        for (int i = 0; i < numLeds; ++i) {
            int sampleIndex = map(i, 0, numLeds - 1, 0, NUM_SAMPLES - 1);
            int brightness = constrain(abs(features.waveform[sampleIndex]) / 256, 0, 255);
            leds[i] = CHSV(map(i, 0, numLeds, 0, 255), 255, brightness);
        }
    }

    const char* getName() const override {
        return "Waveform Bounce";
    }

private:
    CRGB* leds;
    int numLeds;
};

class FrequencyRiver : public BaseAnimation {
public:
    FrequencyRiver(CRGB* buffer, int count) : leds(buffer), numLeds(count) {}

    void begin() override {
        fill_solid(leds, numLeds, CRGB::Black);
    }

    void update(const AudioFeatures& features) override {
        fill_solid(leds, numLeds, CRGB::Black);
        int third = numLeds / 3;
        float bassVal = features.bass * 255;
        float midVal = features.mid * 255;
        float trebVal = features.treble * 255;

        for (int i = 0; i < third; ++i) {
            leds[i] = CHSV(160, 255, bassVal);
        }
        for (int i = third; i < 2 * third; ++i) {
            leds[i] = CHSV(96, 255, midVal);
        }
        for (int i = 2 * third; i < numLeds; ++i) {
            leds[i] = CHSV(0, 255, trebVal);
        }
    }

    const char* getName() const override {
        return "Frequency River";
    }

private:
    CRGB* leds;
    int numLeds;
};

// --- Shared Animation Registry ---

inline std::vector<AnimationFactory> getAnimationRegistry() {
    return {
        [](CRGB* l, int n) -> BaseAnimation* { return new WaveformBounce(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new FrequencyRiver(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new FirestormAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new RippleCascadeAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new ColorTunnelAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new EnergySwirlAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new StrobeMatrixAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new BassBloomAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new ColorDripAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new FrequencyRiverAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new PartyPulseAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new CyberFluxAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new BioSignalAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new ChaosEngineAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new GalacticDriftAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new AudioStormAnimation(l, n); },
        [](CRGB* l, int n) -> BaseAnimation* { return new SpectrumWavesAnimation(l, n); },
    };
}