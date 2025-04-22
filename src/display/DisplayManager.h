#pragma once

#include <TFT_eSPI.h>
#include "../audio/AudioProcessor.h"
#include "GridLayout.h"
#include "themes/ColorTheme.h"

// Forward declarations
class HybridController;

class DisplayManager {
private:
    TFT_eSPI& _tft;
    GridLayout layout;

    // Setting screen state
    bool showSettingScreen = false;
    bool loading = true;
    String activeSettingName = "";
    int activeSettingValue = 0;
    unsigned long settingDisplayTime = 0;
    WidgetColorTheme theme;  
public:
    DisplayManager(TFT_eSPI& display);

    void setTheme(const WidgetColorTheme& newTheme);
    void begin();
    void showStartupScreen();
    void update(const AudioFeatures& features, int animIndex, int animCount, bool autoSwitch, String keepReason);
    void updateAudioVisualization(const AudioFeatures& features, int animIndex, int animCount, bool autoSwitch, String keepReason);
    void showSetting(const String& name, int value);
    void drawSettingScreen();
};
