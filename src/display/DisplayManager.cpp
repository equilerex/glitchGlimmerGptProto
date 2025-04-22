#include "DisplayManager.h"
#include "../core/Debug.h"
#include "widgets/AcronymValueWidget.h"
#include "widgets/VerticalBarWidget.h"
#include "widgets/WaveformWidget.h"
#include <TFT_eSPI.h>
#include "../config/Config.h"
#include "SettingIconRenderer.h"
#include "themes/ColorTheme.h"

DisplayManager::DisplayManager(TFT_eSPI &display)
    : _tft(display), layout(DISPLAY_WIDTH, DISPLAY_HEIGHT),
      showSettingScreen(false), settingDisplayTime(0), loading(true) {
    // Initialize with a default theme
    theme = CyberpunkTheme;
}

void DisplayManager::setTheme(const WidgetColorTheme& newTheme) {
    theme = newTheme;
}

void DisplayManager::showStartupScreen() {
    _tft.fillScreen(TFT_BLACK);
    _tft.setTextColor(theme.primary, TFT_BLACK);
    _tft.setTextSize(2);
    _tft.setCursor(20, 30);
    _tft.print("GlitchGlimmer");
    delay(2000); // Wait for 2 seconds
    _tft.setTextSize(1);
    _tft.setCursor(20, 60);
    _tft.print("Loading visual cortex...");
    delay(1000); // Wait for 2 seconds
    loading = false;
}

void DisplayManager::begin() {
    _tft.init();
    pinMode(DISPLAY_PIN, OUTPUT);
    digitalWrite(DISPLAY_PIN, HIGH);
    delay(1000); // Wait for 2 seconds
    Serial.println("Display initialized");

    _tft.setRotation(1);
    _tft.fillScreen(TFT_BLACK);
    showStartupScreen();
}

void DisplayManager::update(const AudioFeatures& features, int animIndex, int animCount, bool autoSwitch, String keepReason) {
    if (loading) return;
    if (showSettingScreen) {
        drawSettingScreen();
    } else {
        updateAudioVisualization(features, animIndex, animCount, autoSwitch, keepReason);
    }
}

void DisplayManager::showSetting(const String& name, int value) {
    showSettingScreen = true;
    settingDisplayTime = millis();
    activeSettingName = name;
    activeSettingValue = value;
}

void DisplayManager::drawSettingScreen() {
    unsigned long elapsed = millis() - settingDisplayTime;
    if (elapsed > 3000) {
        showSettingScreen = false;
        return;
    }

    float pulse = 1.0 + 0.1 * sin(elapsed / 150.0);
    _tft.fillScreen(TFT_BLACK);

    String icon = activeSettingName;
    if (icon == "BRIGHT") icon = "\xF0\x9F\x94\x8A";
    else if (icon == "SPEED") icon = "\xE2\x9A\xA1";
    else if (icon == "HUE") icon = "\xF0\x9F\x8C\x88";
    else if (icon == "SAT") icon = "\xF0\x9F\x92\xA1";

    _tft.setTextSize(2);
    _tft.setTextColor(theme.primary, TFT_BLACK);
    _tft.setCursor(10, _tft.height() / 2 - 10); _tft.print("<");
    _tft.setCursor(_tft.width() - 20, _tft.height() / 2 - 10); _tft.print(">");

    _tft.setTextColor(theme.primary, TFT_BLACK);
    _tft.setTextSize(2);
    int nameWidth = _tft.textWidth(icon);
    _tft.setCursor((_tft.width() - nameWidth) / 2, 40);
    _tft.print(icon);

    int size = (4 + round((pulse - 1.0) * 8));
    _tft.setTextSize(size);
    _tft.setTextColor(theme.primary, TFT_BLACK);
    String valStr = String(activeSettingValue);
    int valWidth = _tft.textWidth(valStr);
    _tft.setCursor((_tft.width() - valWidth) / 2, _tft.height() / 2 + 20);
    _tft.print(valStr);

    _tft.setTextSize(1);
    _tft.setTextColor(theme.secondary, TFT_BLACK);
    _tft.setCursor((_tft.width() - _tft.textWidth("press knob for more")) / 2, _tft.height() - 16);
    _tft.print("press knob for more");
}

void DisplayManager::updateAudioVisualization(const AudioFeatures& features, int animIndex, int animCount, bool autoSwitch, String keepReason) {
    Debug::log(Debug::DEBUG, "Drawing new frame");
    Debug::logf(Debug::DEBUG, "Features: vol=%.3f, bass=%.3f, mid=%.3f, treb=%.3f, beat=%d, bpm=%.2f, loud=%d",
                features.volume, features.bass, features.mid, features.treble,
                features.beatDetected, features.bpm, features.loudness);

    _tft.fillScreen(TFT_BLACK);
    layout = GridLayout(_tft.width(), _tft.height());

    layout.addWidget(new VerticalBarWidget("BASS", features.bass, theme.bassColor));
    layout.addWidget(new VerticalBarWidget("MID", features.mid, theme.midColor));
    layout.addWidget(new VerticalBarWidget("TREB", features.treble, theme.trebleColor));
    layout.addWidget(new VerticalBarWidget("PWR", features.loudness / 100.0f, theme.powerColor));

    layout.addWidget(new AcronymValueWidget("BPM", static_cast<int>(features.bpm), features.beatDetected));
    layout.addWidget(new AcronymValueWidget("PWR", static_cast<int>(features.loudness)));
    layout.addWidget(new AcronymValueWidget("IDX", animIndex ? animIndex + 1 : 0));
    layout.addWidget(new AcronymValueWidget("TOT", animCount ? animCount : 0));
    layout.addWidget(new AcronymValueWidget("AUTO", autoSwitch));
    layout.addWidget(new AcronymValueWidget("KEEP", String(keepReason)));

    layout.addWidget(new WaveformWidget(features.waveform, NUM_SAMPLES, theme.waveformColor));
    layout.draw(_tft);
}
