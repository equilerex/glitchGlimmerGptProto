// MainController.h
#pragma once

#include <vector>
#include <FastLED.h>
#include <TFT_eSPI.h>

#include "../audio/AudioProcessor.h"
#include "SettingsManager.h"
#include "AnimationManager.h"
#include "../display/DisplayManager.h"
#include "../input/EncoderInput.h"
#include "../input/ButtonInput.h"
#include "../net/WebUI.h"
#include "../animations/LEDStrip.h"

/**
 * MainController orchestrates the overall GlitchGlimmer system.
 * It ties together audio analysis, LED animations, display updates, inputs, and settings management.
 */
class MainController {
public:
    MainController();
    ~MainController();  // Add destructor

    void begin();
    void update();
    void reset();

    void setupStrips();

private:
    TFT_eSPI tft;
    AudioProcessor* audioProcessor;
    SettingsManager settingsManager;
    AnimationManager* animationManager;
    HybridController* hybridController;
    DisplayManager* displayManager;
    EncoderInput* encoderInput;
    ButtonInput* buttonInput;
    WebUI* webUI;

    std::vector<LEDStrip> strips;
};
