#pragma once

#include <TFT_eSPI.h>
#include <vector>
#include "../audio/AudioFeatures.h"
#include "widgets/Widget.h"
#include "themes/ColorTheme.h"
#include "GridLayout.h"
#include "../core/SettingsManager.h"

class DisplayManager {
private:
    TFT_eSPI& tft;
    GridLayout layout;
    WidgetColorTheme theme = CyberpunkTheme;
    std::vector<Widget*> widgets;
    int scrollOffset = 0;
    unsigned long lastScrollTime = 0;
    String currentAnimation = "";
    String currentError = "";

public:
    DisplayManager(TFT_eSPI& display) 
        : tft(display), layout(display.width(), display.height()) {}

    void begin() {
        tft.fillScreen(theme.bg);
        tft.setRotation(1);
        tft.setTextSize(1);
        tft.setTextColor(theme.text);
        layout.clear();
    }

    void setTheme(const WidgetColorTheme& newTheme) {
        theme = newTheme;
    }

    void showStartupScreen() {
        tft.fillScreen(theme.bg);
        tft.setCursor(20, 30);
        tft.setTextColor(theme.primary);
        tft.setTextSize(2);
        tft.print("GlitchGlimmer");
    }

    void update(const AudioFeatures& features, const String& animName,
                int animIndex, int totalAnimations, bool hybridMode,
                const String& modeReason) {
        
        
        layout.addWidget(new VerticalBarWidget("BASS", features.bass, theme.bassColor));
        layout.addWidget(new VerticalBarWidget("MID", features.mid, theme.midColor));
        layout.addWidget(new VerticalBarWidget("TREBLE", features.treble, theme.trebleColor));
        layout.addWidget(new VerticalBarWidget("PWR", features.volume, theme.powerColor));

        layout.addWidget(new WaveformWidget(features.waveform, features.waveformSize, theme, features.beatDetected));
        layout.addWidget(new AcronymValueWidget("MODE", hybridMode ? 1 : 0, hybridMode));
        layout.addWidget(new AcronymValueWidget("IDX", animIndex));
        layout.addWidget(new AcronymValueWidget("MAX", totalAnimations));
        layout.addWidget(new AcronymValueWidget("KEEP", modeReason)); // modeReason is already a String

        //layout.addWidget(new ScrollingTextWidget(animName));

        if (!currentError.isEmpty()) {
            //layout.addWidget(new ScrollingTextWidget(currentError));
        }

        layout.update(tft); // Pass the TFT_eSPI object
    }

    void showSetting(Setting setting, int value) {
        String name = "---";
        switch (setting) {
            case Setting::BRIGHTNESS: name = "BRI"; break;
            case Setting::SPEED:      name = "SPD"; break;
            case Setting::HUE:        name = "HUE"; break;
            case Setting::SATURATION: name = "SAT"; break;
            default: break;
        }
        layout.addWidget(new AcronymValueWidget(name, value, true));
        layout.update(tft); // Pass the TFT_eSPI object
    }

    void drawSettingScreen() {
        layout.update(tft); // Pass the TFT_eSPI object
    }

    void updateAudioVisualization(const AudioFeatures& features,
                                  int animIndex, int totalAnimations,
                                  bool hybridMode, const String& modeReason) {
        update(features, currentAnimation, animIndex, totalAnimations, hybridMode, modeReason);
    }

    void showError(const String& message) {
        currentError = message;
    }

    void clearError() {
        currentError = "";
    }

    void setCurrentAnimation(const String& name) {
        currentAnimation = name;
    }
}; // Add semicolon here