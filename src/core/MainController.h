#pragma once

#include <FastLED.h>
#include <TFT_eSPI.h>
#include "../audio/AudioProcessor.h"
#include "../input/EncoderInput.h"
#include "../input/ButtonInput.h"
#include "../display/DisplayManager.h"
#include "../control/HybridController.h"
#include "../core/LEDStripController.h"
#include "../core/SettingsManager.h"

class MainController {
private:
    AudioProcessor audioProcessor;
    AudioFeatures audioFeatures;
    LEDStripController ledController;
    HybridController hybridController;
    EncoderInput encoderInput;
    ButtonInput buttonInput;
    TFT_eSPI tft;
    DisplayManager displayManager;

public:
    MainController()
        : ledController(audioFeatures),
          hybridController(&ledController),
          encoderInput(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BTN_PIN, settingsManager),
          buttonInput(hybridController, BUTTON_PIN_1, BUTTON_PIN_2),
          displayManager(tft)
    {}

    void begin() {
        tft.init();
        tft.setRotation(1);
        tft.fillScreen(TFT_BLACK);

        displayManager.begin();
        audioProcessor.begin();
        encoderInput.begin();
        buttonInput.begin();
        FastLED.setBrightness(DEFAULT_BRIGHTNESS);
        FastLED.clear();
        FastLED.show();
    }

    void update() {
        audioProcessor.captureAudio();
        audioFeatures = audioProcessor.analyzeAudio();

        encoderInput.update();
        buttonInput.update();
        hybridController.update(audioFeatures);

        ledController.update();

        displayManager.update(
            audioFeatures,
            String(""),
            1,
            4,
            hybridController.isAutoSwitchEnabled(),
            hybridController.getModeKeepReasonText()
        );
    }
};
