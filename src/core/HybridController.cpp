#include "HybridController.h"

HybridController::HybridController(AnimationManager* manager)
    : animationManager(*manager) {
    lastSwitchTime = millis();
}

HybridController::HybridController(AnimationManager& manager)
    : animationManager(manager) {
    lastSwitchTime = millis();
}

void HybridController::update() {
    // Default implementation that doesn't use audio features
    // This creates minimal features for use in the full update method
    AudioFeatures emptyFeatures = {};
    update(emptyFeatures);
}

bool HybridController::isBuildUp() {
    float delta = volumeHistory[(volumePos + 9) % 10] - volumeHistory[(volumePos + 5) % 10];
    buildUp = delta > 0.1;
    return buildUp;
}

bool HybridController::isDrop() {
    float delta = volumeHistory[(volumePos + 9) % 10] - volumeHistory[(volumePos + 5) % 10];
    drop = delta < -0.15;
    return drop;
}

bool HybridController::shouldSwitch(const AudioFeatures& features) {
    if (!autoSwitch) {
        currentReason = ModeKeepReason::DISABLED_;
        return false;
    }

    unsigned long now = millis();
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
        debounceCounter = 0;
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

void HybridController::performSwitch() {
    animationManager.next();
    lastSwitchTime = millis();
    debounceCounter = 0;
    DEBUG_PRINTLN("[HybridController] Switched animation.");
}

void HybridController::update(const AudioFeatures& features) {
    // Smooth volume with a low-pass filter
    const float alpha = 0.2f;
    smoothedVolume = alpha * features.volume + (1.0f - alpha) * smoothedVolume;

    volumeHistory[volumePos] = smoothedVolume;
    volumePos = (volumePos + 1) % 10;

    avgVolume = 0;
    for (int i = 0; i < 10; i++) {
        avgVolume += volumeHistory[i];
    }
    avgVolume /= 10.0;

    if (shouldSwitch(features)) {
        performSwitch();
    }
}

void HybridController::render(const AudioFeatures& features) {
    animationManager.update(features);
}

void HybridController::next() {
    animationManager.next();
    lastSwitchTime = millis();
    currentReason = ModeKeepReason::NONE;
}

void HybridController::previous() {
    animationManager.previous();
    lastSwitchTime = millis();
    currentReason = ModeKeepReason::NONE;
}

void HybridController::toggleAuto() {
    autoSwitch = !autoSwitch;
}

bool HybridController::isAutoSwitchEnabled() const {
    return autoSwitch;
}

const String HybridController::getCurrentName() const {
    return animationManager.getCurrentName();
}

int HybridController::getCurrentIndex() const {
    return animationManager.getCurrentIndex();
}

int HybridController::getAnimationCount() const {
    return animationManager.getAnimationCount();
}

ModeKeepReason HybridController::getModeKeepReason() const {
    return currentReason;
}

String HybridController::getModeKeepReasonText() const {
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
