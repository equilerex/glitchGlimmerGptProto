#pragma once
#include "Widget.h"

class VerticalBarWidget : public Widget {
private:
    String label;
    float normalizedValue;
    uint16_t barColor;

public:
    VerticalBarWidget(String l, float val, uint16_t col = TFT_GREEN)
        : label(l), normalizedValue(val), barColor(col) {}

    void draw(TFT_eSPI& tft, int x, int y, int width, int height) override {
        int barHeight = normalizedValue * height;
        tft.fillRect(x, y, width, height, TFT_DARKGREY);
        tft.fillRect(x, y + height - barHeight, width, barHeight, barColor);

        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(1);
        tft.setRotation(1);  // Vertical text
        tft.setCursor(y + 4, x + 4);
        tft.print(label);
        tft.setRotation(0);
    }

    int getMinWidth() const override { return 135; }
    int getMinHeight() const override { return 30; }
};
