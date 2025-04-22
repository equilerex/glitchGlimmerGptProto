#pragma once

#include <map>

enum class Setting {
    BRIGHTNESS,
    SPEED,
    HUE,
    SATURATION,
    COUNT
};

class SettingsManager {
private:
    std::map<Setting, int> values;
    Setting currentSetting;

public:
    SettingsManager();
    int get(Setting setting) const;
    void set(Setting setting, int value);
    void adjust(int delta);
    void next();
    Setting getCurrentSetting() const;
};