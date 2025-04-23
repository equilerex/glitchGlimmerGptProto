#pragma once


#include <FastLED.h>
#include <TFT_eSPI.h>
#include "../audio/AudioProcessor.h"
#include "../audio/AudioHistoryTracker.h"
#include "../input/EncoderInput.h"
#include "../input/ButtonInput.h"
#include "../display/DisplayManager.h"
#include "../core/LEDStripController.h"
#include "../core/SettingsManager.h"


class MainController {
private:
    AudioFeatures audioFeatures;        // Must come before ledController
    AudioHistoryTracker audioHistory;   // Stores recent audio snapshots
    AudioProcessor audioProcessor;
    LEDStripController ledController;   // Uses audioFeatures and audioHistory
    TFT_eSPI tft;                        // Must come before displayManager
    EncoderInput encoderInput;
    ButtonInput buttonInput;
    DisplayManager displayManager;

public:
    MainController()
        : ledController(audioFeatures),
          encoderInput(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BTN_PIN, settingsManager),
          buttonInput(ledController, BUTTON_PIN_1, BUTTON_PIN_2), 
          displayManager(tft)
    {}

    void begin() {
        tft.init();
        tft.setRotation(1);
        tft.fillScreen(TFT_BLACK);

        audioProcessor.begin();
        displayManager.begin();
        ledController.begin();
        encoderInput.begin();
        buttonInput.begin();
        FastLED.setBrightness(DEFAULT_BRIGHTNESS);
        FastLED.clear();
        FastLED.show();
    }

    void update() {
        // Capture audio input
        audioProcessor.captureAudio();

        // Analyze and store into audioFeatures
        audioFeatures = audioProcessor.analyzeAudio();

        // Track the current frame in rolling history
        audioHistory.addSnapshot(audioFeatures); // <-- Use correct method

        // Update all components
        encoderInput.update();
        buttonInput.update();
        ledController.update();

        displayManager.update(
            audioFeatures,
            String(""),
            1,
            4,
            false
        );

        FastLED.show();
        delay(60);
    }
};
