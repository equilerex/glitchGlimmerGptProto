#pragma once

#include "../audio/AudioFeatures.h"
#include "../core/Debug.h"
#include "core/LEDStripController.h"

enum class ModeKeepReason {
    NONE,
    NO_BEAT,
    TOO_EARLY,
    UNSTABLE_BEAT,
    BUILDUP,
    RECENT_DROP,
    DISABLED_,
    STABLE_BEAT
};

class HybridController {
public:
    HybridController(LEDStripController* controller)
        : ledController(*controller), lastSwitchTime(millis()) {}

    void update() {
        AudioFeatures empty = {};
        update(empty);
    }

    void update(const AudioFeatures& features) {
        const float alpha = 0.2f;
        smoothedVolume = alpha * features.volume + (1.0f - alpha) * smoothedVolume;

        volumeHistory[volumePos] = smoothedVolume;
        volumePos = (volumePos + 1) % HISTORY_SIZE;

        avgVolume = 0;
        for (int i = 0; i < HISTORY_SIZE; i++) {
            avgVolume += volumeHistory[i];
        }
        avgVolume /= HISTORY_SIZE;

        if (shouldSwitch(features)) {
            performSwitch();
        }
    }

    bool shouldSwitch(const AudioFeatures& features) {
        Debug::logf(Debug::DEBUG, "Switch check - Auto: %d, BPM: %.1f", autoSwitch, features.bpm);
        if (!autoSwitch) {
            Debug::log(Debug::DEBUG, "Auto-switch disabled");
            currentReason = ModeKeepReason::DISABLED_;
            return false;
        }

        unsigned long now = millis();
        unsigned long timeSinceLastSwitch = now - lastSwitchTime;
        Debug::logf(Debug::DEBUG, "Time since last switch: %lu ms", timeSinceLastSwitch);

        float bpm = features.bpm > 0 ? features.bpm : 120;
        const unsigned long ABS_MIN = 6000;
        unsigned long beatDuration = 1000 * (60.0 / bpm) * 8;
        unsigned long requiredDelay = max(ABS_MIN, beatDuration);

        bool enoughTimePassed = (now - lastSwitchTime) > requiredDelay;

        if (features.beatDetected) {
            debounceCounter++;
        } else {
            debounceCounter = 0;
            currentReason = ModeKeepReason::NO_BEAT;
        }

        bool beatStable = debounceCounter >= 3;

        if (isBuildUp()) {
            Debug::log(Debug::DEBUG, "Build-up detected, holding pattern");
            currentReason = ModeKeepReason::BUILDUP;
            return false;
        }

        if (isDrop() && (now - lastSwitchTime) < 10000) {
            debounceCounter = 0;
            currentReason = ModeKeepReason::RECENT_DROP;
            return false;
        }

        if (!enoughTimePassed) {
            currentReason = ModeKeepReason::TOO_EARLY;
            return false;
        }

        if (!beatStable) {
            currentReason = ModeKeepReason::UNSTABLE_BEAT;
            return false;
        }

        currentReason = ModeKeepReason::STABLE_BEAT;
        return true;
    }

    bool isBuildUp() const {
        float delta = volumeHistory[(volumePos + HISTORY_SIZE - 1) % HISTORY_SIZE] -
                      volumeHistory[(volumePos + HISTORY_SIZE - 5) % HISTORY_SIZE];
        return delta > 0.1f;
    }

    bool isDrop() const {
        float delta = volumeHistory[(volumePos + HISTORY_SIZE - 1) % HISTORY_SIZE] -
                      volumeHistory[(volumePos + HISTORY_SIZE - 5) % HISTORY_SIZE];
        return delta < -0.15f;
    }

    void performSwitch() {
        ledController.switchAllAnimations();

        lastSwitchTime = millis();
        debounceCounter = 0;
    }

    void next() {
        ledController.switchAllAnimations();
        lastSwitchTime = millis();
        currentReason = ModeKeepReason::NONE;
    }


    void toggleAuto() { autoSwitch = !autoSwitch; }
    bool isAutoSwitchEnabled() const { return autoSwitch; }


    ModeKeepReason getModeKeepReason() const { return currentReason; }
    String getModeKeepReasonText() const {
        switch (currentReason) {
            case ModeKeepReason::NO_BEAT: return "No beat";
            case ModeKeepReason::TOO_EARLY: return "Too early";
            case ModeKeepReason::UNSTABLE_BEAT: return "Unstable beat";
            case ModeKeepReason::BUILDUP: return "Build-up";
            case ModeKeepReason::RECENT_DROP: return "Recent drop";
            case ModeKeepReason::DISABLED_: return "Auto disabled";
            case ModeKeepReason::STABLE_BEAT: return "Stable beat + time";
            default: return "None";
        }
    }

private:
    LEDStripController& ledController;
    bool autoSwitch = true;
    unsigned long lastSwitchTime;
    int debounceCounter = 0;

    static constexpr int HISTORY_SIZE = 10;
    float volumeHistory[HISTORY_SIZE] = {0};
    int volumePos = 0;
    float smoothedVolume = 0.0f;
    float avgVolume = 0.0f;

    ModeKeepReason currentReason = ModeKeepReason::NONE;
};
