#include "AnimationManager.h"
#include "../animations/AnimationRegistry.h"
#include "../utils/Debug.h"


void AnimationManager::initialize() {
    auto all = getAllAnimations();
    for (auto& anim : all) {
        registerAnimation(std::move(anim));
    }
}

void AnimationManager::registerAnimation(std::unique_ptr<BaseAnimation> anim) {
    animations.push_back(std::move(anim));

    DEBUG_PRINTF("Added animation: %d\n",
        anim->getName());
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
    if (!animations.empty()) {
        animations[currentIndex]->update(features);
    }
}

const String AnimationManager::getCurrentName() const {
    return animations.empty() ? "None" : animations[currentIndex]->getName();
}

int AnimationManager::getCurrentIndex() const {
    return currentIndex;
}

int AnimationManager::getAnimationCount() const {
    return animations.size();
}
