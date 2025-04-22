#pragma once

#include <vector>
#include <memory>
#include <TFT_eSPI.h>
#include "widgets/Widget.h"

class GridLayout {
private:
    int _width, _height;
    static constexpr size_t MAX_WIDGETS = 16;
    std::vector<std::unique_ptr<Widget>> widgets;

public:
    GridLayout(int screenWidth, int screenHeight) : _width(screenWidth), _height(screenHeight) {}

    void clear() {
        widgets.clear();
    }

    void addWidget(Widget* widget) {
        if (widgets.size() < MAX_WIDGETS && widget != nullptr) {
            widgets.push_back(std::unique_ptr<Widget>(widget));
        }
    }

    void draw(TFT_eSPI& tft) {
        int x = 0, y = 0;
        int rowHeight = 0;

        for (size_t i = 0; i < widgets.size(); ++i) {
            Widget* widget = widgets[i].get();
            if (!widget) continue;

            int w = widget->getMinWidth();
            int h = widget->getMinHeight();

            if (x + w > _width) {
                x = 0;
                y += rowHeight;
                rowHeight = 0;
            }

            widget->draw(tft, x, y, w, h);
            x += w;
            if (h > rowHeight) rowHeight = h;
        }
    }

    void drawVerticalStack(TFT_eSPI& tft) {
        int y = 0;
        int widgetWidth = _width;
        const int margin = 2;

        for (size_t i = 0; i < widgets.size(); ++i) {
            Widget* widget = widgets[i].get();
            if (!widget) continue;

            int widgetHeight = widget->getMinHeight();
            widget->draw(tft, 0, y, widgetWidth, widgetHeight);
            y += widgetHeight + margin;
        }
    }
};
