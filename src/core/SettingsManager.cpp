#include <Arduino.h> // For constrain
#include "SettingsManager.h"
#include "Debug.h"

SettingsManager::SettingsManager() {
    values[Setting::BRIGHTNESS] = 128;
    values[Setting::SPEED] = 100;
    values[Setting::HUE] = 0;
    values[Setting::SATURATION] = 255;
    currentSetting = Setting::BRIGHTNESS;
}

int SettingsManager::get(Setting setting) const {
    return values.at(setting);
}

void SettingsManager::set(Setting setting, int value) {
    values[setting] = value;
    Debug::logf(Debug::DEBUG, "SettingsManager: Set %d to %d\n", static_cast<int>(setting), value);
}

void SettingsManager::adjust(int delta) {
    values[currentSetting] = constrain(values[currentSetting] + delta, 0, 255);
    Debug::logf(Debug::DEBUG, "SettingsManager: Adjusted %d to %d\n", static_cast<int>(currentSetting), values[currentSetting]);
}

void SettingsManager::next() {
    currentSetting = static_cast<Setting>((static_cast<int>(currentSetting) + 1) % static_cast<int>(Setting::COUNT));

    Debug::logf(Debug::DEBUG, "SettingsManager: Switched to setting %d",  static_cast<int>(currentSetting));
}

Setting SettingsManager::getCurrentSetting() const {
    return currentSetting;
}
