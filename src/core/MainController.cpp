// MainController.cpp
#include <TFT_eSPI.h>
#include "MainController.h"
#include "../animations/Animations.h"
#include "../core/SettingsManager.h"
#include "../display/DisplayManager.h"
#include "../net/WebUI.h"
#include "Debug.h"
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
    Debug::log(Debug::INFO, "MainController: Starting initialization");

    // Setup display and display manager
    displayManager.begin();
    Debug::log(Debug::INFO, "Display initialized");

    // Start audio and input processors
    try {
        audioProcessor.begin();
        Debug::log(Debug::INFO, "Audio processor initialized");
    } catch (const std::exception& e) {
        Debug::logf(Debug::ERROR, "Audio initialization failed: %s", e.what());
        throw;
    }

    // Setup LED strips using config defines
    setupStrips();

    try {
        // Initialize animation manager
        animationManager.initialize();
        Debug::log(Debug::INFO, "Animation manager initialized");

        buttonInput.begin();
        encoderInput.begin();
        Debug::log(Debug::INFO, "Input devices initialized");
    } catch (const std::exception& e) {
        Debug::logf(Debug::ERROR, "Component initialization failed: %s", e.what());
        throw;
    }

    Debug::log(Debug::INFO, "MainController initialization complete");
}

void MainController::update() {
    #if ENABLE_HEAP_MONITORING
    static unsigned long lastHeapCheck = 0;
    if (millis() - lastHeapCheck > 5000) {  // Check every 5 seconds
        size_t freeHeap = ESP.getFreeHeap();
        Debug::logf(Debug::DEBUG, "Free heap: %d bytes", freeHeap);
        lastHeapCheck = millis();
        
        if (freeHeap < MIN_FREE_HEAP) {
            Debug::log(Debug::ERROR, "Critical: Low memory condition");
        }
    }
    #endif

    audioProcessor.captureAudio();
    AudioFeatures features = audioProcessor.analyzeAudio();

    Debug::logf(Debug::DEBUG, "[MainController] AudioFeatures: vol=%.3f, bass=%.3f, mid=%.3f, treb=%.3f, beat=%d, bpm=%.2f, loud=%d\n",
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
    Debug::log(Debug::INFO, "Setting up LED strips");
    
    size_t freeHeap = ESP.getFreeHeap();
    Debug::logf(Debug::INFO, "Free heap before LED init: %d bytes", freeHeap);
    
    if (freeHeap < MIN_FREE_HEAP) {
        Debug::log(Debug::ERROR, "Not enough memory to initialize LED strips");
        return;
    }

#ifdef LED_STRIP_COUNT_1
    try {
        CRGB* leds1 = new CRGB[LED_STRIP_COUNT_1];
        if (!leds1) {
            Debug::log(Debug::ERROR, "Failed to allocate memory for LED strip 1");
            return;
        }
        FastLED.addLeds<WS2812B, LED_STRIP_PIN_1, RGB>(leds1, LED_STRIP_COUNT_1);
        FastLED.setBrightness(DEFAULT_BRIGHTNESS);
        strips.emplace_back(leds1, LED_STRIP_COUNT_1);
        strips.back().begin();
        Debug::logf(Debug::INFO, "LED Strip 1 initialized: %d LEDs", LED_STRIP_COUNT_1);
    } catch (const std::exception& e) {
        Debug::logf(Debug::ERROR, "LED Strip 1 init failed: %s. Free heap: %d", 
                    e.what(), ESP.getFreeHeap());
        throw;
    }
#endif

#ifdef LED_STRIP_COUNT_2
    CRGB* leds2 = new CRGB[LED_STRIP_COUNT_2];
    FastLED.addLeds<LED_STRIP_TYPE_2, LED_STRIP_PIN_2, GRB>(leds2, LED_STRIP_COUNT_2);
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
    strips.emplace_back(leds2, LED_STRIP_COUNT_2);
    strips.back().begin();
#endif
}
