#pragma once

#include <FastLED.h>
#include <functional>
#include <array>
#include "../audio/AudioFeatures.h"
 

#ifdef LED_0_PIN
CRGB ledStrip_0[LED_0_NUM];
#endif
#ifdef LED_1_PIN
CRGB ledStrip_1[LED_1_NUM];
#endif
#ifdef LED_2_PIN
CRGB ledStrip_2[LED_2_NUM];
#endif
#ifdef LED_3_PIN
CRGB ledStrip_3[LED_3_NUM];
#endif
#ifdef LED_4_PIN
CRGB ledStrip_4[LED_4_NUM];
#endif
#ifdef LED_5_PIN
CRGB ledStrip_5[LED_5_NUM];
#endif
#ifdef LED_6_PIN
CRGB ledStrip_6[LED_6_NUM];
#endif
#ifdef LED_7_PIN
CRGB ledStrip_7[LED_7_NUM];
#endif
#ifdef LED_8_PIN
CRGB ledStrip_8[LED_8_NUM];
#endif
#ifdef LED_9_PIN
CRGB ledStrip_9[LED_9_NUM];
#endif

enum class AnimationType {
    RED_PULSE,
    BLUE_PULSE,
    GREEN_PULSE,
    COUNT
};

class Animation {
public:
    virtual ~Animation() = default;
    virtual void begin() {}
    virtual void update(CRGB* leds, int n, const AudioFeatures& features) = 0;
};

using AnimationFactory = std::function<Animation*()>;

class RedPulseAnimation : public Animation {
public:
    void update(CRGB* leds, int n, const AudioFeatures& audio) override {
        uint8_t brightness = max(15, static_cast<int>(audio.volume * 255));
        fill_solid(leds, n, CHSV(0, 255, brightness));
    }
};

class BluePulseAnimation : public Animation {
public:
    void update(CRGB* leds, int n, const AudioFeatures& audio) override {
        uint8_t brightness = max(10, static_cast<int>(audio.volume * 255));
        fill_solid(leds, n, CHSV(160, 255, brightness));
    }
};

class GreenPulseAnimation : public Animation {
public:
    void update(CRGB* leds, int n, const AudioFeatures& audio) override {
        uint8_t brightness = max(20, static_cast<int>(audio.volume * 255));
        fill_solid(leds, n, CHSV(96, 255, brightness));
    }
};

static const std::array<AnimationFactory, static_cast<size_t>(AnimationType::COUNT)> animationFactories = {
    []() { return new RedPulseAnimation(); },
    []() { return new BluePulseAnimation(); },
    []() { return new GreenPulseAnimation(); }
};

class LEDStrip {
public:
    int index;
    int length;
    CRGB* leds;
    Animation* currentAnimation;

    LEDStrip() : index(-1), length(0), leds(nullptr), currentAnimation(nullptr) {}
    ~LEDStrip() { if (currentAnimation) delete currentAnimation; }

    void init(int len, CRGB* buffer, AnimationType type, AudioFeatures& audio) {
        length = len;
        leds = buffer;
        setAnimation(type, audio);
    }

    void setAnimation(AnimationType type, AudioFeatures&) {
        if (currentAnimation) delete currentAnimation;
        int typeIndex = static_cast<int>(type);
        currentAnimation = animationFactories[typeIndex]();
    }

    void update(const AudioFeatures& audio) {
        if (currentAnimation && leds) currentAnimation->update(leds, length, audio);
    }

    const char* getName() const {
        return "Unnamed"; // You may customize this if needed
    }
};

class LEDStripController {
private:
    AudioFeatures& audio;
    LEDStrip strips[10];
    int stripCount = 0;
    int currentPatternIndex[10] = {0};

public:
    LEDStripController(AudioFeatures& audioRef) : audio(audioRef) {
        FastLED.clear();
#ifdef LED_0_PIN
        FastLED.addLeds<WS2812B, LED_0_PIN, GRB>(ledStrip_0, LED_0_NUM);
        strips[stripCount++].init(LED_0_NUM, ledStrip_0, AnimationType::RED_PULSE, audio);
#endif
#ifdef LED_1_PIN
        FastLED.addLeds<WS2812B, LED_1_PIN, GRB>(ledStrip_1, LED_1_NUM);
        strips[stripCount++].init(LED_1_NUM, ledStrip_1, AnimationType::BLUE_PULSE, audio);
#endif
#ifdef LED_2_PIN
        FastLED.addLeds<WS2812B, LED_2_PIN, GRB>(ledStrip_2, LED_2_NUM);
        strips[stripCount++].init(LED_2_NUM, ledStrip_2, AnimationType::GREEN_PULSE, audio);
#endif
#ifdef LED_3_PIN
        FastLED.addLeds<WS2812B, LED_3_PIN, GRB>(ledStrip_3, LED_3_NUM);
        strips[stripCount++].init(LED_3_NUM, ledStrip_3, AnimationType::RED_PULSE, audio);
#endif
#ifdef LED_4_PIN
        FastLED.addLeds<WS2812B, LED_4_PIN, GRB>(ledStrip_4, LED_4_NUM);
        strips[stripCount++].init(LED_4_NUM, ledStrip_4, AnimationType::BLUE_PULSE, audio);
#endif
#ifdef LED_5_PIN
        FastLED.addLeds<WS2812B, LED_5_PIN, GRB>(ledStrip_5, LED_5_NUM);
        strips[stripCount++].init(LED_5_NUM, ledStrip_5, AnimationType::GREEN_PULSE, audio);
#endif
#ifdef LED_6_PIN
        FastLED.addLeds<WS2812B, LED_6_PIN, GRB>(ledStrip_6, LED_6_NUM);
        strips[stripCount++].init(LED_6_NUM, ledStrip_6, AnimationType::RED_PULSE, audio);
#endif
#ifdef LED_7_PIN
        FastLED.addLeds<WS2812B, LED_7_PIN, GRB>(ledStrip_7, LED_7_NUM);
        strips[stripCount++].init(LED_7_NUM, ledStrip_7, AnimationType::BLUE_PULSE, audio);
#endif
#ifdef LED_8_PIN
        FastLED.addLeds<WS2812B, LED_8_PIN, GRB>(ledStrip_8, LED_8_NUM);
        strips[stripCount++].init(LED_8_NUM, ledStrip_8, AnimationType::GREEN_PULSE, audio);
#endif
#ifdef LED_9_PIN
        FastLED.addLeds<WS2812B, LED_9_PIN, GRB>(ledStrip_9, LED_9_NUM);
        strips[stripCount++].init(LED_9_NUM, ledStrip_9, AnimationType::RED_PULSE, audio);
#endif
        FastLED.setBrightness(DEFAULT_BRIGHTNESS);
        FastLED.show();
    }

    void update() {
        for (int i = 0; i < stripCount; ++i) strips[i].update(audio);
        FastLED.show();
    }

    void switchAllAnimations() {
        for (int i = 0; i < stripCount; ++i) {
            int next = (currentPatternIndex[i] + 1) % static_cast<int>(AnimationType::COUNT);
            currentPatternIndex[i] = next;
            strips[i].setAnimation(static_cast<AnimationType>(next), audio);
        }
    }

    int getActiveAnimationIndex(int i) const {
        return i < stripCount ? currentPatternIndex[i] : -1;
    }

    int getAnimationCount() const {
        return static_cast<int>(AnimationType::COUNT);
    }

    int getStripCount() const {
        return stripCount;
    }

    int getTotalAnimations(int) const {
        return static_cast<int>(AnimationType::COUNT);
    }

    int getCurrentAnimationName(int i) const {
        return getActiveAnimationIndex(i);
    }
};
