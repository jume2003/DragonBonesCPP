#include "Animation.h"

NAME_SPACE_DRAGON_BONES_BEGIN

bool Animation::getIsPlaying() const
{
    return _isPlaying && !getIsComplete();
}
bool Animation::getIsComplete() const
{
    if (_lastAnimationState)
    {
        if (!_lastAnimationState->_isComplete)
        {
            return false;
        }
        
        for (size_t i = 0, l = _animationStateList.size(); i < l; ++i)
        {
            if (!_animationStateList[i]->_isComplete)
            {
                return false;
            }
        }
        
        return true;
    }
    
    return true;
}

AnimationState *Animation::getLastAnimationState() const
{
    return _lastAnimationState;
}

/*
const std::vector<String>& Animation::getAnimationList() const
{
    const std::vector<String>& animationList = std::vector<String>
    for (size_t i = 0, l = _animationStateList.size(); i < l; ++i)
    {
        if(!_animationStateList[i]->_isComplete)
        {
            return false;
        }
    }
    }*/

float Animation::getTimeScale() const
{
    return _timeScale;
}
void Animation::setTimeScale(float timeScale)
{
    if (timeScale < 0 || timeScale != timeScale)
    {
        timeScale = 1;
    }
    
    _timeScale = timeScale;
}

Animation::Animation()
{
    _isPlaying = false;
    autoTween = true;
    _timeScale = 1.f;
    _armature = nullptr;
    _lastAnimationState = nullptr;
}
Animation::~Animation()
{
    dispose();
}

void Animation::dispose()
{
    animationDataList.clear();
    
    for (size_t i = 0, l = _animationStateList.size(); i < l; ++i)
    {
        AnimationState::returnObject(_animationStateList[i]);
    }
    
    _animationStateList.clear();
    _armature = nullptr;
    _lastAnimationState = nullptr;
}

AnimationState *Animation::gotoAndPlay(
    const String &animationName,
    float fadeInTime,
    float duration,
    int playTimes,
    int layer,
    const String &group,
    const AnimationFadeOutMode &fadeOutMode,
    bool pauseFadeOut,
    bool pauseFadeIn
)
{
    AnimationData *animationData = nullptr;
    
    for (size_t i = 0, l = animationDataList.size(); i < l; ++i)
    {
        if (animationDataList[i]->name == animationName)
        {
            animationData = animationDataList[i];
            break;
        }
    }
    
    if (!animationData)
    {
        // throw
        return nullptr;
    }
    
    _isPlaying = true;
    _isFading = true;
    fadeInTime = fadeInTime < 0 ? (animationData->fadeTime < 0 ? 0.3f : animationData->fadeTime) : fadeInTime;
    float durationScale;
    
    if (duration < 0)
    {
        durationScale = animationData->scale < 0 ? 1.f : animationData->scale;
    }
    
    else
    {
        durationScale = duration * 1000 / animationData->duration;
    }
    
    playTimes = playTimes < 0 ? animationData->playTimes : playTimes;
    
    switch (fadeOutMode)
    {
        case AnimationFadeOutMode::NONE:
            break;
            
        case AnimationFadeOutMode::SAME_LAYER:
            for (size_t i = 0, l = _animationStateList.size(); i < l; ++i)
            {
                AnimationState *animationState = _animationStateList[i];
                
                if (animationState->_layer == layer)
                {
                    animationState->fadeOut(fadeInTime, pauseFadeOut);
                }
            }
            
            break;
            
        case AnimationFadeOutMode::SAME_GROUP:
            for (size_t i = 0, l = _animationStateList.size(); i < l; ++i)
            {
                AnimationState *animationState = _animationStateList[i];
                
                if (animationState->_group == group)
                {
                    animationState->fadeOut(fadeInTime, pauseFadeOut);
                }
            }
            
            break;
            
        case AnimationFadeOutMode::ALL:
            for (size_t i = 0, l = _animationStateList.size(); i < l; ++i)
            {
                AnimationState *animationState = _animationStateList[i];
                animationState->fadeOut(fadeInTime, pauseFadeOut);
            }
            
            break;
            
        case AnimationFadeOutMode::SAME_LAYER_AND_GROUP:
        default:
            for (size_t i = 0, l = _animationStateList.size(); i < l; ++i)
            {
                AnimationState *animationState = _animationStateList[i];
                
                if (animationState->_layer == layer && animationState->_group == group)
                {
                    animationState->fadeOut(fadeInTime, pauseFadeOut);
                }
            }
            
            break;
    }
    
    _lastAnimationState = AnimationState::borrowObject();
    _lastAnimationState->_layer = layer;
    _lastAnimationState->_group = group;
    _lastAnimationState->autoTween = autoTween;
    _lastAnimationState->fadeIn(_armature, animationData, fadeInTime, 1.f / durationScale, playTimes, pauseFadeIn);
    addState(_lastAnimationState);
    const auto &slotList = _armature->getSlots();
    
    for (size_t i = 0, l = slotList.size(); i < l; ++i)
    {
        Slot *slot = slotList[i];
        
        if (slot->getChildArmature())
        {
            slot->getChildArmature()->_animation->gotoAndPlay(animationName, fadeInTime);
        }
    }
    
    return _lastAnimationState;
}

AnimationState *Animation::gotoAndStop(
    const String &animationName,
    float time,
    float normalizedTime,
    float fadeInTime,
    float duration,
    int layer,
    const String &group,
    const AnimationFadeOutMode &fadeOutMode
)
{
    AnimationState *animationState = getState(animationName, layer);
    
    if (!animationState)
    {
        animationState = gotoAndPlay(animationName, fadeInTime, duration, -1, layer, group, fadeOutMode);
    }
    
    if (normalizedTime >= 0)
    {
        animationState->setCurrentTime(animationState->getTotalTime() * normalizedTime);
    }
    
    else
    {
        animationState->setCurrentTime(time);
    }
    
    animationState->stop();
    return animationState;
}

void Animation::play()
{
    if (animationDataList.empty())
    {
        return;
    }
    
    if (!_lastAnimationState)
    {
        gotoAndPlay(animationDataList[0]->name);
    }
    
    else if (!_isPlaying)
    {
        _isPlaying = true;
    }
    
    else
    {
        gotoAndPlay(_lastAnimationState->name);
    }
}

void Animation::stop()
{
    _isPlaying = false;
}

bool Animation::hasAnimation(const String &animationName) const
{
    for (size_t i = 0, l = animationDataList.size(); i < l; ++i)
    {
        if (animationDataList[i]->name == animationName)
        {
            return true;
        }
    }
    
    return false;
}

AnimationState *Animation::getState(const String &name, int layer) const
{
    for (size_t i = 0, l = _animationStateList.size(); i < l; ++i)
    {
        AnimationState *animationState = _animationStateList[i];
        
        if (animationState->name == name && animationState->_layer == layer)
        {
            return animationState;
        }
    }
    
    return nullptr;
}

void Animation::advanceTime(float passedTime)
{
    if (!_isPlaying)
    {
        return;
    }
    
    bool isFading = false;
    passedTime *= _timeScale;
    
    // reverse advance
    for (size_t i = _animationStateList.size(); i--;)
    {
        AnimationState *animationState = _animationStateList[i];
        
        if (animationState->advanceTime(passedTime))
        {
            removeState(animationState);
        }
        
        else if (animationState->_fadeState != AnimationState::FadeState::FADE_COMPLETE)
        {
            isFading = true;
        }
    }
    
    _isFading = isFading;
}

void Animation::updateAnimationStates()
{
    for (size_t i = 0, l = _animationStateList.size(); i < l; ++i)
    {
        _animationStateList[i]->updateTimelineStates();
    }
}

void Animation::addState(AnimationState *animationState)
{
    auto iterator = std::find(_animationStateList.cbegin(), _animationStateList.cend(), animationState);
    
    if (iterator == _animationStateList.cend())
    {
        _animationStateList.push_back(animationState);
    }
}

void Animation::removeState(AnimationState *animationState)
{
    auto iterator = std::find(_animationStateList.begin(), _animationStateList.end(), animationState);
    
    if (iterator != _animationStateList.end())
    {
        _animationStateList.erase(iterator);
        AnimationState::returnObject(animationState);
        
        if (_lastAnimationState == animationState)
        {
            if (_animationStateList.empty())
            {
                _lastAnimationState = nullptr;
            }
            
            else
            {
                _lastAnimationState = _animationStateList.back();
            }
        }
    }
}
NAME_SPACE_DRAGON_BONES_END