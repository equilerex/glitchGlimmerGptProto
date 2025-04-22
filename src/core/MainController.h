#pragma once

#include <TFT_eSPI.h>
#include "../core/SettingsManager.h"
#include "../display/DisplayManager.h"
#include "../net/WebUI.h"
#include "../config/Config.h"
#include "../core/Debug.h"
#include "../led/LEDGlobals.h"
#include "../pipeline/AnimationPipeline.h"
#include "../audio/AudioProcessor.h"
#include "../input/EncoderInput.h"
#include "../input/ButtonInput.h"
#include "../control/HybridController.h"
#include <FastLED.h>

extern SettingsManager settingsManager;

class MainController {
public:
    MainController()
        : tft(),
          audioProcessor(),
          hybridController1(&animationManager1),
          hybridController2(&animationManager2),
          displayManager(tft),
          encoderInput(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BTN_PIN, settingsManager),
          buttonInput(&hybridController1, BUTTON_PIN_1, BUTTON_PIN_2),
          webUI() {
        encoderInput.setDisplay(&displayManager);
    }

    void begin() {
        Debug::log(Debug::INFO, "MainController: Starting initialization");

        displayManager.begin();
        Debug::log(Debug::INFO, "Display initialized");

        setupLEDs();
        Debug::log(Debug::INFO, "LED strips initialized");

        audioProcessor.begin();
        Debug::log(Debug::INFO, "Audio processor initialized");

        std::vector<AnimationFactory> registry = getAnimationRegistry();
        animationManager1.setup(leds1, LED_STRIP_COUNT_1);
        animationManager2.setup(leds2, LED_STRIP_COUNT_2);
        animationManager1.initialize(registry);
        animationManager2.initialize(registry);
        Debug::log(Debug::INFO, "Animations initialized");

        buttonInput.begin();
        encoderInput.begin();
        Debug::log(Debug::INFO, "Input devices initialized");

        Debug::log(Debug::INFO, "MainController initialization complete");
    }

    void update() {
        audioProcessor.captureAudio();
        AudioFeatures features = audioProcessor.analyzeAudio();

        Debug::logf(Debug::DEBUG,
            "[MainController] AudioFeatures: vol=%.3f, bass=%.3f, mid=%.3f, treb=%.3f, beat=%d, bpm=%.2f, loud=%d",
            features.volume, features.bass, features.mid, features.treble,
            features.beatDetected, features.bpm, features.loudness);

        buttonInput.update();
        encoderInput.update();
        hybridController1.update(features);
        hybridController2.update(features);

        animationManager1.update(features);
        animationManager2.update(features);

        FastLED.show();

        displayManager.update(features,
                              animationManager1.getCurrentName(),
                              animationManager1.getCurrentIndex(),
                              animationManager1.getAnimationCount(),
                              hybridController1.isAutoSwitchEnabled(),
                              hybridController1.getModeKeepReasonText());
    }

private:
    TFT_eSPI tft;
    AudioProcessor audioProcessor;
    DisplayManager displayManager;
    EncoderInput encoderInput; 

    AnimationManager animationManager1;
    AnimationManager animationManager2;

    AudioFeatures features;

    HybridController hybridController1;
    HybridController hybridController2;
    ButtonInput buttonInput;
    WebUI webUI;
};
