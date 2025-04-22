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
    : tft() {
    audioProcessor = new AudioProcessor();
    animationManager = new AnimationManager();
    hybridController = new HybridController(animationManager);
    displayManager = new DisplayManager(tft);
    encoderInput = new EncoderInput(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BTN_PIN, settingsManager);
    buttonInput = new ButtonInput(hybridController, BUTTON_PIN_1, BUTTON_PIN_2);
    webUI = new WebUI();
    
    encoderInput->setDisplay(displayManager);
}

MainController::~MainController() {
    delete audioProcessor;
    delete animationManager;
    delete hybridController;
    delete displayManager;
    delete encoderInput;
    delete buttonInput;
    delete webUI;
}

void MainController::begin() {
    Debug::log(Debug::INFO, "MainController: Starting initialization");

    try {
        // Setup display first for debug output
        displayManager->begin();
        Debug::log(Debug::INFO, "Display initialized");

        // Initialize LED strips before audio to ensure proper memory allocation
        setupStrips();
        Debug::log(Debug::INFO, "LED strips initialized");

        // Then initialize audio processor
        audioProcessor->begin();
        Debug::log(Debug::INFO, "Audio processor initialized");

        // Initialize remaining components
        animationManager->initialize();
        Debug::log(Debug::INFO, "Animation manager initialized");

        buttonInput->begin();
        encoderInput->begin();
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

    // Track if we need to reset the audio system
    static int consecutiveZeroAudio = 0;
    static unsigned long lastAudioResetTime = 0;

    audioProcessor->captureAudio();
    AudioFeatures features = audioProcessor->analyzeAudio();

    // Check if we're getting audio data
    if (features.volume < 0.001) {
        consecutiveZeroAudio++;
        if (consecutiveZeroAudio > 10 && (millis() - lastAudioResetTime > 10000)) {
            Debug::log(Debug::INFO, "No audio data detected, resetting I2S interface");
            audioProcessor->resetI2S();
            lastAudioResetTime = millis();
            consecutiveZeroAudio = 0;
        }
    } else {
        consecutiveZeroAudio = 0;
    }

    Debug::logf(Debug::DEBUG, "[MainController] AudioFeatures: vol=%.3f, bass=%.3f, mid=%.3f, treb=%.3f, beat=%d, bpm=%.2f, loud=%d\n",
        features.volume, features.bass, features.mid, features.treble,
        features.beatDetected, features.bpm, features.loudness);


    // Inputs
    buttonInput->update();
    encoderInput->update();
    hybridController->update();
    // No settingsManager.update() needed - it's handled by encoderInput

    // Update animation with audio features
    animationManager->update(features);
    
    // Show updated animations on all LED strips
    for (auto& strip : strips) {
        strip.show();
    }

    // Display UI
    displayManager->update(features, 
                         animationManager->getCurrentName(),
                         animationManager->getCurrentIndex(),
                         animationManager->getAnimationCount(),
                         hybridController->isAutoSwitchEnabled(),
                         hybridController->getModeKeepReasonText());

    // Optional Web Control
   // webUI.update();
}

void MainController::reset() {
    Debug::log(Debug::INFO, "Resetting MainController state");
    
    // Delete existing instances
    delete audioProcessor;
    delete animationManager;
    delete hybridController;
    delete displayManager;
    delete encoderInput;
    delete buttonInput;
    delete webUI;
    
    // Recreate instances
    audioProcessor = new AudioProcessor();
    animationManager = new AnimationManager();
    hybridController = new HybridController(animationManager);
    displayManager = new DisplayManager(tft);
    encoderInput = new EncoderInput(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BTN_PIN, settingsManager);
    buttonInput = new ButtonInput(hybridController, BUTTON_PIN_1, BUTTON_PIN_2);
    webUI = new WebUI();
    
    encoderInput->setDisplay(displayManager);
    
    // Initialize components
    displayManager->begin();
    setupStrips();
    audioProcessor->begin();
    
    Debug::log(Debug::INFO, "MainController reset complete");
}

void MainController::setupStrips() {
    Debug::log(Debug::INFO, "Setting up LED strips");
    
    #ifdef LED_STRIP_PIN_1 
    try {
        Debug::logf(Debug::DEBUG, "Initializing LED strip 1 with %d LEDs", LED_STRIP_COUNT_1);
        
        // Static allocation for LED array
        static CRGB leds1[LED_STRIP_COUNT_1];
        
        // Configure FastLED
        FastLED.addLeds<WS2812B, LED_STRIP_PIN_1, RGB>(leds1, LED_STRIP_COUNT_1);
        // Create strip object
        strips.emplace_back(leds1, LED_STRIP_COUNT_1);
        strips.back().begin();
        
        Debug::log(Debug::DEBUG, "Setting initial state");

        
        Debug::log(Debug::INFO, "LED Strip 1 initialized successfully");
    } catch (const std::exception& e) {
        Debug::logf(Debug::ERROR, "LED Strip 1 initialization failed: %s", e.what());
        throw;
    }
    #endif

 
    FastLED.show();
}
