#pragma once

#include <TFT_eSPI.h>
#include "../audio/AudioFeatures.h"
#include "../core/SettingsManager.h"
#include "../control/HybridController.h"
#include "../display/widgets/Widget.h" 
#include "GridLayout.h"

class DisplayManager {
public:
    DisplayManager(TFT_eSPI& display) : tft(display), layout(display) {}

    void begin() {
        tft.init();
        tft.setRotation(1);
        tft.fillScreen(TFT_BLACK);

        layout.begin();
        layout.setPadding(4);

        // Shared theme
        WidgetColorTheme theme;

        // Add basic indicator widgets
        layout.addWidget(new AcronymValueWidget("VOL", [] { return static_cast<int>(currentFeatures.volume * 100); }));
        layout.addWidget(new AcronymValueWidget("PWR", [] { return currentFeatures.loudness; }));
        layout.addWidget(new AcronymValueWidget("BPM", [] { return static_cast<int>(currentFeatures.bpm); }));
        layout.addWidget(new AcronymValueWidget("BT", [] { return currentFeatures.beatDetected ? 1 : 0; }));

        // Add vertical bars for bands
        layout.addWidget(new VerticalBarWidget("BASS", [] { return currentFeatures.bass; }));
        layout.addWidget(new VerticalBarWidget("MID", [] { return currentFeatures.mid; }));
        layout.addWidget(new VerticalBarWidget("TREB", [] { return currentFeatures.treble; }));

        // Waveform
        layout.addWidget(new WaveformWidget([] { return currentFeatures.waveform; }, NUM_SAMPLES, theme, true));
    }

    void update(const AudioFeatures& features,
                const String& currentAnimationName,
                int currentIndex,
                int total,
                bool autoMode,
                const String& modeKeepReason) {
        currentFeatures = features;
        currentAnimName = currentAnimationName;
        currentAutoMode = autoMode;
        currentModeReason = modeKeepReason;

        layout.update();
    }

    void showSetting(Setting setting, int value) {
        layout.setTemporaryOverride([=](TFT_eSPI& tft) {
            tft.fillScreen(TFT_BLACK);
            tft.setCursor(0, 20);
            tft.setTextColor(TFT_YELLOW);
            tft.setTextSize(2);
            tft.printf("%s\n%d", settingToLabel(setting), value);
        });
    }

private:
    const char* settingToLabel(Setting s) {
        switch (s) {
            case Setting::BRIGHTNESS: return "Brightness";
            case Setting::SPEED: return "Speed";
            case Setting::HUE: return "Hue";
            case Setting::SATURATION: return "Saturation";
            default: return "Unknown";
        }
    }

    TFT_eSPI& tft;
    GridLayout layout;

    static inline AudioFeatures currentFeatures = {};
    static inline String currentAnimName = "";
    static inline bool currentAutoMode = false;
    static inline String currentModeReason = "";
};
