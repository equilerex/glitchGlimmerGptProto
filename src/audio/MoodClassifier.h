// MoodClassifier.h
#pragma once
#include <String.h>
#include "AudioHistoryTracker.h"

class MoodClassifier {
public:
    String classify(const std::deque<AudioSnapshot>& recent) {
        if (recent.empty()) return "Unknown";

        float avgVol = 0, avgEnergy = 0, avgCentroid = 0, beatRate = 0;
        int beatCount = 0;

        for (const auto& snap : recent) {
            avgVol += snap.volume;
            avgEnergy += snap.energy;
            avgCentroid += snap.centroid;
            if (snap.beat) beatCount++;
        }

        int n = recent.size();
        avgVol /= n;
        avgEnergy /= n;
        avgCentroid /= n;
        beatRate = beatCount * 1000.0f / (recent.back().timestamp - recent.front().timestamp + 1);

        if (avgVol < 0.1 && avgEnergy < 50) return "Calm";
        if (avgCentroid > 150 && avgEnergy > 1000 && avgVol > 0.5) return "Drop";
        if (avgEnergy > 900 && beatRate > 1.5f) return "Groove";
        if (avgEnergy > 800 && avgCentroid > 120) return "Buildup";
        if (avgEnergy > 1100 && avgCentroid > 140) return "Chaos";

        return "Idle";
    }
};
