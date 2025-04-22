#include "AnimationManager.h"
#include "../animations/AnimationRegistry.h"
#include "Debug.h"


void AnimationManager::initialize() {
    Debug::log(Debug::INFO, "AnimationManager::initialize starting");

    try {
        auto all = getAllAnimations();
        Debug::logf(Debug::INFO, "Created %d animations", all.size());

        for (auto& anim : all) {
            Debug::logf(Debug::DEBUG, "Animation object at address: 0x%p", (void*)anim.get());

            if (anim) {
                const char* name = anim->getName();
                if (name) {
                    Debug::logf(Debug::INFO, "Registering animation: %s", name);
                } else {
                    Debug::log(Debug::ERROR, "Animation has null name");
                }
                registerAnimation(std::move(anim));
            } else {
                Debug::log(Debug::ERROR, "Null animation encountered during initialization");
            }
        }
    } catch (const std::exception& e) {
        Debug::logf(Debug::ERROR, "Exception in AnimationManager::initialize: %s", e.what());
    } catch (...) {
        Debug::log(Debug::ERROR, "Unknown exception in AnimationManager::initialize");
    }

    Debug::logf(Debug::INFO, "AnimationManager::initialize complete, %d animations registered", animations.size());
}

void AnimationManager::registerAnimation(std::unique_ptr<BaseAnimation> anim) {
    if (!anim) {
        Debug::log(Debug::ERROR, "Attempted to register null animation");
        return;
    }

    // Safely get the name with null check
    const char* animName = "Unknown";
    try {
        if (anim) {
            animName = anim->getName();
            if (!animName) animName = "Null Name";
        }
    } catch (...) {
        Debug::log(Debug::ERROR, "Exception getting animation name");
    }

    Debug::logf(Debug::INFO, "Registering animation: %s [0x%p]", animName, (void*)anim.get());
    animations.push_back(std::move(anim));
    
    // Check if the animation was stored successfully
    if (animations.back()) {
        // Safely get the name with null check
        const char* storedName = "Unknown";
        try {
            storedName = animations.back()->getName();
            if (!storedName) storedName = "Null Name";
        } catch (...) {
            Debug::log(Debug::ERROR, "Exception getting stored animation name");
        }
        
        Debug::logf(Debug::INFO, "Added animation: %s at index %d", storedName, animations.size() - 1);
    } else {
        Debug::log(Debug::ERROR, "Animation became null after push_back");
    }
    
    Debug::logf(Debug::INFO, "AnimationManager::initialize complete, %d animations registered", animations.size());
 
}

void AnimationManager::next() {
    if (!animations.empty()) {
        currentIndex = (currentIndex + 1) % animations.size();
    }
}

void AnimationManager::previous() {
    if (!animations.empty()) {
        currentIndex = (currentIndex - 1 + animations.size()) % animations.size();
    }
}

void AnimationManager::update(const AudioFeatures& features) {
    if (animations.empty()) {
        Debug::log(Debug::ERROR, "Cannot update animation: no animations registered");
        return;
    }

    // Bounds check the current index
    if (currentIndex < 0 || currentIndex >= animations.size()) {
        Debug::logf(Debug::ERROR, "Animation index out of bounds: %d (max: %d)",
                    currentIndex, animations.size() - 1);
        currentIndex = 0; // Reset to a safe value
    }

    // Check for null pointer before updating
    auto& currentAnim = animations[currentIndex];
    if (!currentAnim) {
        Debug::logf(Debug::ERROR, "Null animation at index %d", currentIndex);
        return;
    }

    try {
        // Log which animation we're updating

            Debug::logf(Debug::DEBUG, "Updating animation: %s", currentAnim->getName());


        // Update the animation
        currentAnim->update(features);
    } catch (const std::exception& e) {
        Debug::logf(Debug::ERROR, "Exception in animation %s update: %s",
                    currentAnim->getName(), e.what());
    } catch (...) {
        Debug::logf(Debug::ERROR, "Unknown exception in animation update at index %d", currentIndex);
    }
}

const String AnimationManager::getCurrentName() const {
    if (animations.empty()) {
        return "None";
    }

    if (currentIndex < 0 || currentIndex >= animations.size()) {
        Debug::logf(Debug::ERROR, "Invalid current index in getCurrentName: %d", currentIndex);
        return "Invalid";
    }

    if (!animations[currentIndex]) {
        Debug::log(Debug::ERROR, "Null animation in getCurrentName");
        return "Null";
    }

    try {
        return animations[currentIndex]->getName();
    } catch (...) {
        Debug::log(Debug::ERROR, "Exception in getName()");
        return "Error";
    }
}

int AnimationManager::getCurrentIndex() const {
    return currentIndex;
}

int AnimationManager::getAnimationCount() const {
    return animations.size();
}
