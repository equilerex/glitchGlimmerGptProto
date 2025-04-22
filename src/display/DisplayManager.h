#pragma once

#include <TFT_eSPI.h>
#include "../audio/AudioFeatures.h"
#include "../core/SettingsManager.h"
#include "../control/HybridController.h"
#include "../display/widgets/Widget.h"
#include "../display/GridLayout.h"

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

        // Add basic indicator widgets with dummy initial values
        layout.addWidget(new AcronymValueWidget("VOL", 0));
        layout.addWidget(new AcronymValueWidget("PWR", 0));
        layout.addWidget(new AcronymValueWidget("BPM", 0));
        layout.addWidget(new AcronymValueWidget("BT", 0));

        // Add vertical bars for bands
        layout.addWidget(new VerticalBarWidget("BASS", 0.0f));
        layout.addWidget(new VerticalBarWidget("MID", 0.0f));
        layout.addWidget(new VerticalBarWidget("TREB", 0.0f));

        // Waveform
        layout.addWidget(new WaveformWidget(nullptr, NUM_SAMPLES, theme, true));
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

    // Remove 'inline' from these static variables
    static AudioFeatures currentFeatures;
    static String currentAnimName;
    static bool currentAutoMode;
    static String currentModeReason;
};
