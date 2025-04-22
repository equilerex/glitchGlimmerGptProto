#pragma once
#include "Widget.h"
#include "../../core/HybridController.h"
#include <TFT_eSPI.h>
#include <Arduino.h>

class AcronymValueWidget : public Widget {
private:
    String label;  // Remove const to prevent String reference issues
    int value;
    bool highlight;

public:
    // Single constructor with defaults
    AcronymValueWidget(const String& l, int v = 1, bool h = false) : 
        label(l), value(v), highlight(h) {
        if (label.isEmpty()) {
            label = "---";  // Ensure label is never empty
        }
    }
    
    // Constructor for ModeKeepReason
    AcronymValueWidget(const String& l, ModeKeepReason reason) :
        label(l), value(static_cast<int>(reason)), highlight(false) {
        if (label.isEmpty()) {
            label = "---";
        }
    }

    // Constructor for boolean values
    AcronymValueWidget(const String& l, bool b) :
        label(l), value(b ? 1 : 0), highlight(false) {
        if (label.isEmpty()) {
            label = "---";
        }
    }

    void draw(TFT_eSPI& tft, int x, int y, int width, int height) override {
        // Background
        const uint16_t bgColor = highlight ? TFT_BLACK: TFT_BLACK;
        tft.fillRoundRect(x, y, width, height, 4, bgColor);

        // Label
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE);
        const int labelPadding = 4;
        tft.setCursor(x + labelPadding, y + labelPadding);
        tft.print(label);

        // Value
        tft.setTextSize(2);
        tft.setTextColor(highlight ? TFT_RED : TFT_CYAN);
        const int valueY = y + (height - 16) / 2; // 16 is text height at size 2
        const int valueX = x + (width - 12) / 2;  // Approximate width of single digit
        tft.setCursor(valueX, valueY);
        tft.print(value);
    }

    int getMinWidth() const override { return 60; }
    int getMinHeight() const override { return 40; }
};
