// MainController.cpp
#include <TFT_eSPI.h>
#include "MainController.h"
#include "../animations/Animations.h"
#include "../core/SettingsManager.h"
#include "../display/DisplayManager.h"
#include "../net/WebUI.h"
#include "../utils/Debug.h"
#include "../config/Config.h"
#include <FastLED.h>



MainController::MainController()
    : tft(), audioProcessor(), settingsManager(), animationManager(), hybridController(&animationManager), displayManager(tft),
      encoderInput(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BTN_PIN, settingsManager),
      buttonInput(&hybridController, BUTTON_PIN_1, BUTTON_PIN_2) {
    // Set display reference for encoder input
    encoderInput.setDisplay(&displayManager);
}

void MainController::begin() { 
    DEBUG_PRINTLN("[MainController] Initializing system...");

    // Setup display and display manager
    displayManager.begin();

    // Start audio and input processors
    audioProcessor.begin();

    // Setup LED strips using config defines
    setupStrips();

    // Initialize animation manager
    animationManager.initialize();

    buttonInput.begin();
    encoderInput.begin();

    // Link Web UI if needed
    //webUI.begin(&settingsManager);

    DEBUG_PRINTLN("[MainController] System initialized successfully.");
}

void MainController::update() {
    audioProcessor.captureAudio();
    AudioFeatures features = audioProcessor.analyzeAudio();

    DEBUG_PRINTF("[MainController] AudioFeatures: vol=%.3f, bass=%.3f, mid=%.3f, treb=%.3f, beat=%d, bpm=%.2f, loud=%d\n",
        features.volume, features.bass, features.mid, features.treble,
        features.beatDetected, features.bpm, features.loudness);

    // Inputs
    buttonInput.update();
    encoderInput.update();
    hybridController.update();
    // No settingsManager.update() needed - it's handled by encoderInput

    // Update animation with audio features
    animationManager.update(features);
    
    // Show updated animations on all LED strips
    for (auto& strip : strips) {
        strip.show();
    }

    // Display UI
    displayManager.update(features, animationManager.getCurrentIndex(),
                         animationManager.getAnimationCount(),
                         hybridController.isAutoSwitchEnabled(),
                         hybridController.getModeKeepReasonText());

    // Optional Web Control
   // webUI.update();
}



void MainController::setupStrips() {
#ifdef LED_STRIP_COUNT_1
    CRGB* leds1 = new CRGB[LED_STRIP_COUNT_1];

    FastLED.addLeds<WS2812B, LED_STRIP_PIN_1, RGB>(leds1, LED_STRIP_COUNT_1);
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
    strips.emplace_back(leds1, LED_STRIP_COUNT_1);
    strips.back().begin();
#endif

#ifdef LED_STRIP_COUNT_2
    CRGB* leds2 = new CRGB[LED_STRIP_COUNT_2];
    FastLED.addLeds<LED_STRIP_TYPE_2, LED_STRIP_PIN_2, GRB>(leds2, LED_STRIP_COUNT_2);
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
    strips.emplace_back(leds2, LED_STRIP_COUNT_2);
    strips.back().begin();
#endif
}
