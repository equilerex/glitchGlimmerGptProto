#pragma once
#include "Widget.h"
#include "../../core/Debug.h"

class WaveformWidget : public Widget {
private:
    const int16_t* waveform;
    int samples;
    WidgetColorTheme theme;
    bool beatPulse;
    float pulseIntensity = 0.0f;
    
    bool isValidWaveform() const {
        if (!waveform) return false;
        if (samples <= 1) return false;
        // Check if pointer is in ESP32's RAM range
        uint32_t ptr = (uint32_t)waveform;
        return (ptr >= 0x3FF80000 && ptr < 0x40000000);
    }

public:
    WaveformWidget(const int16_t* wf, int samp, const WidgetColorTheme& themeRef, bool pulseOnBeat = false)
        : waveform(wf), samples(samp), theme(themeRef), beatPulse(pulseOnBeat) {
        Debug::logPointer(Debug::DEBUG, "WaveformWidget", wf, __FILE__, __LINE__);
    }

    void draw(TFT_eSPI& tft, int x, int y, int width, int height) override {
        // Draw border
        tft.drawRect(x, y, width, height, theme.secondary);
        
        if (!isValidWaveform()) {
            drawNoSignal(tft, x, y, width, height);
            return;
        }

        int baseY = y + height / 2;
        uint16_t waveColor = beatPulse ? 
            theme.powerColor : 
            theme.primary;

        // Update pulse effect
        if (beatPulse) {
            pulseIntensity = min(1.0f, pulseIntensity + 0.2f);
        } else {
            pulseIntensity = max(0.0f, pulseIntensity - 0.1f);
        }

        // Draw waveform with interpolation
        int lastY = baseY;
        for (int i = 0; i < width; i++) {
            float idx = (float)i * samples / width;
            int index = (int)idx;
            float frac = idx - index;
            
            // Interpolate between samples
            float sample;
            if (index < samples - 1) {
                sample = waveform[index] * (1.0f - frac) + waveform[index + 1] * frac;
            } else {
                sample = waveform[index];
            }
            
            int y1 = map(sample, -32768, 32767, -height/2, height/2);
            int currentY = baseY + y1;
            
            // Draw vertical line for the waveform
            tft.drawLine(x + i, baseY, x + i, currentY, waveColor);
            
            // Connect points with line
            if (i > 0) {
                tft.drawLine(x + i - 1, lastY, x + i, currentY, waveColor);
            }
            lastY = currentY;
        }

        // Draw pulse overlay if active
        if (pulseIntensity > 0) {
            uint8_t alpha = pulseIntensity * 128;
            tft.drawRect(x, y, width, height, theme.powerColor);
        }
    }

private:
    void drawNoSignal(TFT_eSPI& tft, int x, int y, int width, int height) {
        // Draw "No Signal" pattern
        int centerX = x + width / 2;
        int centerY = y + height / 2;
        
        // Draw crossed lines
        tft.drawLine(x, y, x + width, y + height, theme.powerColor);
        tft.drawLine(x, y + height, x + width, y, theme.powerColor);
        
        // Draw "No Audio" text
        tft.setTextColor(theme.powerColor);
        tft.setTextSize(1);
        tft.setCursor(centerX - 20, centerY - 3);
        tft.print("No Audio");
    }

public:
    int getMinWidth() const override { return 100; }
    int getMinHeight() const override { return 20; }
 
    int getTypeId() const { return 3; }

    void triggerPulse() {
        // ...existing code...
    }
};
