#pragma once

#include "BaseAnimation.h"
#include "../core/Debug.h"

// Include all animations
#include "animations.h"

// Add new animation includes here as needed...

#include <vector>
#include <memory>
#include "../config/Config.h"  // Include for make_unique fallback implementation

// Helper to get animation name at compile time
template<class AnimationType>
constexpr const char* getAnimationName() {
    return AnimationType::staticName();
}

// Safe wrapper to create animation with error handling
template<class AnimationType>
inline std::unique_ptr<BaseAnimation> createAnimationSafe() {
    try {
        auto animation = make_unique<AnimationType>();
        if (!animation) {
            Debug::logAnimationError(getAnimationName<AnimationType>(), "allocation failed");
            return nullptr;
        }
        return animation;
    } catch (const std::exception& e) {
        Debug::logAnimationError(getAnimationName<AnimationType>(), e.what());
        return nullptr;
    } catch (...) {
        Debug::logAnimationError(getAnimationName<AnimationType>(), "unknown error");
        return nullptr;
    }
}

inline std::vector<std::unique_ptr<BaseAnimation>> getAllAnimations() {
    Debug::log(Debug::INFO, "Creating all animations");
    std::vector<std::unique_ptr<BaseAnimation>> animations;
    
    // Add animations with error handling
    // This lets us continue loading other animations even if one fails
    try {
        // Try to preallocate memory to avoid reallocation errors
        animations.reserve(15);
        
        auto addAnimation = [&animations](std::unique_ptr<BaseAnimation> anim) {
            if (anim) {
                animations.push_back(std::move(anim));
            }
        };

        addAnimation(createAnimationSafe<FirestormAnimation>());
        addAnimation(createAnimationSafe<RippleCascadeAnimation>());
        addAnimation(createAnimationSafe<ColorTunnelAnimation>());
        addAnimation(createAnimationSafe<EnergySwirlAnimation>());
        addAnimation(createAnimationSafe<StrobeMatrixAnimation>());
        addAnimation(createAnimationSafe<BassBloomAnimation>());
        addAnimation(createAnimationSafe<ColorDripAnimation>());
        addAnimation(createAnimationSafe<FrequencyRiverAnimation>());
        addAnimation(createAnimationSafe<PartyPulseAnimation>());
        addAnimation(createAnimationSafe<CyberFluxAnimation>());
        addAnimation(createAnimationSafe<BioSignalAnimation>());
        addAnimation(createAnimationSafe<ChaosEngineAnimation>());
        addAnimation(createAnimationSafe<GalacticDriftAnimation>());
        addAnimation(createAnimationSafe<AudioStormAnimation>());
        addAnimation(createAnimationSafe<SpectrumWavesAnimation>());
        // addAnimation(createAnimationSafe<BassPulseAnimation>());
        
    } catch (const std::exception& e) {
        Debug::logf(Debug::ERROR, "Exception in getAllAnimations: %s", e.what());
    } catch (...) {
        Debug::log(Debug::ERROR, "Unknown exception in getAllAnimations");
    }
    
    Debug::logf(Debug::INFO, "Successfully created %d animations", animations.size());
    return animations;
}
