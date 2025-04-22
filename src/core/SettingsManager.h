#pragma once

#include <Arduino.h>
#include <map>
#include "Debug.h"
#include "../config/Config.h"

enum class Setting {
    BRIGHTNESS = DEFAULT_BRIGHTNESS,
    SPEED,
    HUE,
    SATURATION,
    COUNT
};

class SettingsManager {
public:
    SettingsManager() {
        values[Setting::BRIGHTNESS] = 128;
        values[Setting::SPEED] = 100;
        values[Setting::HUE] = 0;
        values[Setting::SATURATION] = 255;
        currentSetting = Setting::BRIGHTNESS;
    }

    int get(Setting setting) const {
        auto it = values.find(setting);
        return it != values.end() ? it->second : 0;
    }

    void set(Setting setting, int value) {
        values[setting] = value;
        Debug::logf(Debug::DEBUG, "SettingsManager: Set %d to %d\n", static_cast<int>(setting), value);
    }

    void adjust(int delta) {
        values[currentSetting] = constrain(values[currentSetting] + delta, 0, 255);
        Debug::logf(Debug::DEBUG, "SettingsManager: Adjusted %d to %d\n", static_cast<int>(currentSetting), values[currentSetting]);
    }

    void next() {
        currentSetting = static_cast<Setting>((static_cast<int>(currentSetting) + 1) % static_cast<int>(Setting::COUNT));
        Debug::logf(Debug::DEBUG, "SettingsManager: Switched to setting %d", static_cast<int>(currentSetting));
    }

    Setting getCurrentSetting() const {
        return currentSetting;
    }

private:
    std::map<Setting, int> values;
    Setting currentSetting;
};

extern SettingsManager settingsManager;
