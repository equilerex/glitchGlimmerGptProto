// MoodHistory.h
#pragma once
#include <Arduino.h>
#include "../audio/AudioFeatures.h"

struct MoodSample {
    unsigned long timestamp;
    float volume;
    float energy;
    float centroid;
};

class MoodHistory {
public:
    static const int MAX_SAMPLES = 120; // Enough for ~1 min at 500ms interval
    MoodSample buffer[MAX_SAMPLES];
    int head = 0;
    int count = 0;

    void add(const AudioFeatures& features) {
        buffer[head] = {
            millis(),
            features.volume,
            features.energy,
            features.spectrumCentroid
        };
        head = (head + 1) % MAX_SAMPLES;
        if (count < MAX_SAMPLES) count++;
    }

    String analyzeMood() const {
        float avgEnergy = 0, avgVolume = 0, avgCentroid = 0;
        for (int i = 0; i < count; ++i) {
            avgEnergy += buffer[i].energy;
            avgVolume += buffer[i].volume;
            avgCentroid += buffer[i].centroid;
        }
        if (count == 0) return "Unknown";
        avgEnergy /= count;
        avgVolume /= count;
        avgCentroid /= count;

        if (avgVolume < 0.15 && avgEnergy < 250) return "Calm";
        if (avgEnergy > 700 && avgCentroid > 80) return "Hype";
        if (avgCentroid > 120) return "Build";
        return "Neutral";
    }
};