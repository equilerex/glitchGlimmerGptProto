#pragma once

#include "VisualLayer.h"
#include <deque>
#include <vector>
#include <FastLED.h>
#include "../audio/AudioSnapshot.h"
#include "../audio/AudioFeatures.h"
#include "../audio/AudioHistoryTracker.h"  // Add this include
#include <Arduino.h> // For Serial

class LayerManager {
private:
    std::vector<VisualLayer*> layers;
    CRGB* compositeBuffer = nullptr;
    int ledCount = 0;

public:
    LayerManager() = default;

    void init(CRGB* buffer, int count) {
        compositeBuffer = buffer;
        ledCount = count;
    }

    void addLayer(VisualLayer* layer) {
        layers.push_back(layer);
        // Log layer addition
        Serial.print("[LayerManager] Added layer: ");
        Serial.println(layer->getName());
    }

    void updateLayers(const AudioFeatures& now, const std::deque<AudioSnapshot>& history) {
        for (auto* layer : layers) {
            Serial.print("[LayerManager] Updating layer: ");
            Serial.println(layer->getName());
            layer->update(now, history);
        }
    }

    // Overload to handle AudioHistoryTracker pointer
    void updateLayers(const AudioFeatures& now, const AudioHistoryTracker* historyTracker) {
        if (historyTracker) {
            updateLayers(now, historyTracker->getHistory());
        }
    }

    void renderLayers() {
        if (compositeBuffer == nullptr || ledCount == 0) return;
        
        fill_solid(compositeBuffer, ledCount, CRGB::Black);
        for (auto* layer : layers) {
            Serial.print("[LayerManager] Rendering layer: ");
            Serial.println(layer->getName());
            CRGB tempBuffer[ledCount];
            fill_solid(tempBuffer, ledCount, CRGB::Black);
            layer->render(tempBuffer, ledCount);

            for (int i = 0; i < ledCount; ++i) {
                compositeBuffer[i] += tempBuffer[i].nscale8_video(static_cast<uint8_t>(layer->opacity * 255));
            }
        }
        FastLED.show();
    }

    // Add overload that accepts specific buffer and length
    void renderLayers(CRGB* leds, int count) {
        if (leds == nullptr || count == 0) return;
        
        fill_solid(leds, count, CRGB::Black);
        for (auto* layer : layers) {
            Serial.print("[LayerManager] Rendering layer (custom buffer): ");
            Serial.println(layer->getName());
            CRGB tempBuffer[count];
            fill_solid(tempBuffer, count, CRGB::Black);
            layer->render(tempBuffer, count);

            for (int i = 0; i < count; ++i) {
                leds[i] += tempBuffer[i].nscale8_video(static_cast<uint8_t>(layer->opacity * 255));
            }
        }
        // Don't call FastLED.show() here to give caller control over timing
    }

    void clearLayers() {
        Serial.println("[LayerManager] Clearing all layers.");
        layers.clear();
    }
};
