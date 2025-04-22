#pragma once
#include <Arduino.h>

class ControllerSettings {
private:
    int brightness = 128;
    int speed = 100;
    int colorShift = 0;

public:
    const char* getSettingName(int index) const {
        switch (index) {
            case 0: return "BRI";
            case 1: return "SPD";
            case 2: return "CLR";
            default: return "???";
        }
    }

    int getSettingValue(int index) const {
        switch (index) {
            case 0: return brightness;
            case 1: return speed;
            case 2: return colorShift;
            default: return 0;
        }
    }

    void modifySetting(int index, int delta) {
        switch (index) {
            case 0: brightness = constrain(brightness + delta, 0, 255); break;
            case 1: speed = constrain(speed + delta, 10, 500); break;
            case 2: colorShift = constrain(colorShift + delta, -100, 100); break;
        }
    }

    int getBrightness() const { return brightness; }
    int getSpeed() const { return speed; }
    int getColorShift() const { return colorShift; }
};
