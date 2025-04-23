#pragma once

#include <FastLED.h>
#include <functional>
#include <array>
#include <deque>
#include "../audio/AudioFeatures.h"
#include "../config/Config.h"
#include "../animations/AlienPulse.h"
#include "../animations/neonFlow.h"
#include "../animations/PsychedelicInkSquirtAnimation.h"
#include "../animations/MultiLayeredHybridAnimation.h"
#include "../animations/LayerManager.h"
#include "../animations/VisualLayers.h"
#include "../audio/AudioHistoryTracker.h"
#include "../animations/Animation.h"

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
    MULTI_LAYERED_HYBRID,
    NEON_FLOW,
    PSYCHEDELIC_INK_SQUIRTS,
    ALIEN_PULSE,
    RED_PULSE,
    BLUE_PULSE,
    GREEN_PULSE,
    COUNT
};

using AnimationFactory = std::function<Animation*()>;

class RedPulseAnimation : public Animation {
public:
    void update(CRGB* leds, int n, const AudioFeatures& audio) override {
        uint8_t brightness = std::max<uint8_t>(15, static_cast<uint8_t>(audio.volume * 255));
        fill_solid(leds, n, CHSV(0, 255, brightness));
    }
};

class BluePulseAnimation : public Animation {
public:
    void update(CRGB* leds, int n, const AudioFeatures& audio) override {
        uint8_t brightness = std::max<uint8_t>(10, static_cast<uint8_t>(audio.volume * 255));
        fill_solid(leds, n, CHSV(160, 255, brightness));
    }
};

class GreenPulseAnimation : public Animation {
public:
    void update(CRGB* leds, int n, const AudioFeatures& audio) override {
        uint8_t brightness = std::max<uint8_t>(20, static_cast<uint8_t>(audio.volume * 255));
        fill_solid(leds, n, CHSV(96, 255, brightness));
    }
};

static const std::array<AnimationFactory, static_cast<size_t>(AnimationType::COUNT)> animationFactories = {

    []() { return new MultiLayeredHybridAnimation(); },
    []() { return new NeonFlowAnimation(); },
    []() { return new PsychedelicInkSquirtAnimation(); },
    []() { return new AlienPulseAnimation(); },
    []() { return new RedPulseAnimation(); },
    []() { return new BluePulseAnimation(); },
    []() { return new GreenPulseAnimation(); }
};

class LEDStrip {
public:
    int index = -1;
    int length = 0;
    CRGB* leds = nullptr;
    Animation* currentAnimation = nullptr;
    LayerManager layerManager;
    const AudioHistoryTracker* sharedHistory = nullptr;

    ~LEDStrip() {
        if (currentAnimation) delete currentAnimation;
        layerManager.clearLayers();
    }

    void setAudioHistory(const AudioHistoryTracker* history) {
        sharedHistory = history;
    }

    void init(int len, CRGB* buffer, AnimationType type, AudioFeatures& audio) {
        length = len;
        leds = buffer;
        setAnimation(type, audio);
        configureDefaultLayers();
    }

    void setAnimation(AnimationType type, AudioFeatures& audio) {
        if (currentAnimation) delete currentAnimation;
        size_t index = static_cast<size_t>(type);
        if (index < animationFactories.size()) {
            currentAnimation = animationFactories[index]();
            if (currentAnimation) currentAnimation->update(leds, length, audio);
        }
    }

    void update(const AudioFeatures& audio) {
        if (currentAnimation && leds) currentAnimation->update(leds, length, audio);
       if (sharedHistory) {
            layerManager.updateLayers(audio, sharedHistory);
            layerManager.renderLayers(leds, length);
        }
    }

    void configureDefaultLayers() {
        int r = random(15);
        switch (r) {
            case 0: layerManager.addLayer(new TriwaveBeatLayer()); break;
            case 1: layerManager.addLayer(new BassShockwaveLayer()); break;
            case 2: layerManager.addLayer(new BPMWavePulseLayer()); break;
            case 3: layerManager.addLayer(new BeatFlashSparkLayer()); break;
            case 4: layerManager.addLayer(new DominantBandTrailLayer()); break;
            case 5: layerManager.addLayer(new CentroidGlowWipeLayer()); break;
            case 6: layerManager.addLayer(new SpectralRibbonLayer()); break;
            case 7: layerManager.addLayer(new MoodMemoryArcLayer()); break;
            case 8: layerManager.addLayer(new LoudnessLightningLayer()); break;
            case 9: layerManager.addLayer(new EnergyFogLayer()); break;
            case 10: layerManager.addLayer(new WormholeVortexLayer()); break;
            case 11: layerManager.addLayer(new CentroidRadianceLayer()); break;
            case 12: layerManager.addLayer(new WaveformScribbleLayer()); break;
            case 13: layerManager.addLayer(new DominantBandFireTrailLayer()); break;
            case 14: layerManager.addLayer(new EnergyPulseRiverLayer()); break;
        }
    }
};

class LEDStripController {
private:
    AudioFeatures& audio;
    LEDStrip strips[10];
    int stripCount = 0;
    int currentPatternIndex[10] = {0};
    AudioHistoryTracker audioHistory;

public:
    LEDStripController(AudioFeatures& audioRef) : audio(audioRef) {}

    void begin() {
        FastLED.clear();
        #ifdef LED_0_PIN
        FastLED.addLeds<WS2812B, LED_0_PIN, GRB>(ledStrip_0, LED_0_NUM);
        strips[stripCount++].init(LED_0_NUM, ledStrip_0, AnimationType::MULTI_LAYERED_HYBRID, audio);
        #endif
        #ifdef LED_1_PIN
        FastLED.addLeds<WS2812B, LED_1_PIN, GRB>(ledStrip_1, LED_1_NUM);
        strips[stripCount++].init(LED_1_NUM, ledStrip_1, AnimationType::PSYCHEDELIC_INK_SQUIRTS, audio);
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

        for (int i = 0; i < stripCount; ++i) {
            strips[i].setAudioHistory(&audioHistory);
        }

        FastLED.setBrightness(DEFAULT_BRIGHTNESS);
        FastLED.show();
    }

    void update() {
        audioHistory.addSnapshot(audio);
        for (int i = 0; i < stripCount; ++i) strips[i].update(audio);
        FastLED.show();
    }

    void switchAllAnimations() {
static const char* animationNames[] = {
            "MULTI_LAYERED_HYBRID",
            "NEON_FLOW",
            "PSYCHEDELIC_INK_SQUIRTS",
            "ALIEN_PULSE",
            "RED_PULSE",
            "BLUE_PULSE",
            "GREEN_PULSE"
        };
        for (int i = 0; i < stripCount; ++i) {
            int next = (currentPatternIndex[i] + 1) % static_cast<int>(AnimationType::COUNT);
            currentPatternIndex[i] = next;

            // Print animation index for debugging
const char* name = (next >= 0 && next < 7) ? animationNames[next] : "UNKNOWN";
            Serial.printf("switchAllAnimations: strip %d, new animation: %d (%s)\n", i, next, name);

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
