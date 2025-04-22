#pragma once

#include <TFT_eSPI.h>
#include <vector>
#include <functional>
#include "widgets/Widget.h"

class GridLayout {
public:
    GridLayout(TFT_eSPI& tft) : display(tft) {}

    void begin() {
        display.fillScreen(TFT_BLACK);
    }

    void setPadding(int p) {
        padding = p;
    }

    void addWidget(Widget* widget) {
        widgets.push_back(widget);
    }

    void setTemporaryOverride(std::function<void(TFT_eSPI&)> drawOverride) {
        overrideDraw = drawOverride;
        overrideUntil = millis() + overrideDuration;
    }

    void update() {
        if (overrideDraw && millis() < overrideUntil) {
            overrideDraw(display);
            return;
        }

        display.fillScreen(TFT_BLACK);

        int columns = 3;
        int rows = (widgets.size() + columns - 1) / columns;

        int cellW = display.width() / columns;
        int cellH = display.height() / rows;

        for (size_t i = 0; i < widgets.size(); ++i) {
            int x = (i % columns) * cellW + padding;
            int y = (i / columns) * cellH + padding;
            int w = cellW - 2 * padding;
            int h = cellH - 2 * padding;
            widgets[i]->draw(display, x, y, w, h);
        }
    }

private:
    TFT_eSPI& display;
    std::vector<Widget*> widgets;
    int padding = 4;

    std::function<void(TFT_eSPI&)> overrideDraw = nullptr;
    unsigned long overrideUntil = 0;
    const unsigned long overrideDuration = 3000;
};
