#include "Animation.h"
#include "../armature/Armature.h"
#include "../armature/Slot.h"
#include "AnimationState.h"

NAMESPACE_DRAGONBONES_BEGIN

bool Animation::_sortAnimationState(AnimationState* a, AnimationState* b)
{
    return a->getLayer() < b->getLayer();
}

Animation::Animation() 
{
    _onClear();
}
Animation::~Animation() 
{
    _onClear();
}

void Animation::_onClear()
{
    timeScale = 1.f;

    _timelineStateDirty = false;
    _armature = nullptr;

    _isPlaying = false;
    _lastAnimationState = nullptr;

    _animations.clear();
    _animationNames.clear();

    for (auto animationState : _animationStates)
    {
        animationState->returnToPool();
    }

    _animationStates.clear();
}

void Animation::_fadeOut(float fadeOutTime, int layer, const std::string& group, AnimationFadeOutMode fadeOutMode, bool pauseFadeOut)
{
    switch (fadeOutMode)
    {
        case AnimationFadeOutMode::None:
            break;

        case AnimationFadeOutMode::SameLayer:
            for (const auto animationState : _animationStates)
            {
                if (animationState->getLayer() == layer)
                {
                    animationState->fadeOut(fadeOutTime, pauseFadeOut);
                }
            }
            break;

        case AnimationFadeOutMode::SameGroup:
            for (const auto animationState : _animationStates)
            {
                if (animationState->getGroup() == group)
                {
                    animationState->fadeOut(fadeOutTime, pauseFadeOut);
                }
            }
            break;

        case AnimationFadeOutMode::All:
            for (const auto animationState : _animationStates)
            {
                animationState->fadeOut(fadeOutTime, pauseFadeOut);
            }
            break;

        case AnimationFadeOutMode::SameLayerAndGroup:
            for (const auto animationState : _animationStates)
            {
                if (animationState->getLayer() == layer && animationState->getGroup() == group)
                {
                    animationState->fadeOut(fadeOutTime, pauseFadeOut);
                }
            }
            break;
    }
}

void Animation::_updateFFDTimelineStates()
{
    for (const auto animationState : _animationStates)
    {
        animationState->_updateFFDTimelineStates();
    }
}

void Animation::_advanceTime(float passedTime)
{
    if (!_isPlaying)
    {
        return;
    }

	if (passedTime < 0.f)
	{
		passedTime = -passedTime;
	}

    const auto animationStateCount = _animationStates.size();
    if (animationStateCount == 1)
    {
        const auto animationState = _animationStates[0];
        if (animationState->_isFadeOutComplete)
        {
            animationState->returnToPool();
            _animationStates.clear();
            _lastAnimationState = nullptr;
        }
        else
        {
            if (_timelineStateDirty)
            {
                animationState->_updateTimelineStates();
            }

            animationState->_advanceTime(passedTime, 1.f, 0);
        }
    }
    else if (animationStateCount > 1)
    {
        auto prevLayer = _animationStates[0]->_layer;
        auto weightLeft = 1.f;
        auto layerTotalWeight = 0.f;
        unsigned layerIndex = 1;

        for (std::size_t i = 0, r = 0; i < animationStateCount; ++i)
        {
            const auto animationState = _animationStates[i];
            if (animationState->_isFadeOutComplete)
            {
                r++;
                animationState->returnToPool();

                if (_lastAnimationState == animationState)
                {
                    if (i - r >= 0)
                    {
                        _lastAnimationState = _animationStates[i - r];
                    }
                    else
                    {
                        _lastAnimationState = nullptr;
                    }
                }
            }
            else
            {
                if (r > 0)
                {
                    _animationStates[i - r] = animationState;
                }

                if (prevLayer != animationState->_layer)
                {
                    prevLayer = animationState->_layer;

                    if (layerTotalWeight >= weightLeft)
                    {
                        weightLeft = 0.f;
                    }
                    else
                    {
                        weightLeft -= layerTotalWeight;
                    }

                    layerTotalWeight = 0.f;
                }

                if (_timelineStateDirty)
                {
                    animationState->_updateTimelineStates();
                }

                animationState->_advanceTime(passedTime, weightLeft, layerIndex);

                if (animationState->_weightResult != 0.f)
                {
                    layerTotalWeight += animationState->_weightResult;
                    layerIndex++;
                }
            }

            if (i == animationStateCount - 1 && r > 0)
            {
                _animationStates.resize(animationStateCount - r);
            }
        }
    }

    _timelineStateDirty = false;
}

void Animation::reset()
{
    _isPlaying = false;
    _lastAnimationState = nullptr;

    for (const auto animationState : _animationStates)
    {
        animationState->returnToPool();
    }

    _animationStates.clear();
}

void Animation::stop()
{
    _isPlaying = false;
}

AnimationState* Animation::play(const std::string& animationName, int playTimes)
{
    AnimationState* animationState = nullptr;
    if (!animationName.empty())
    {
        animationState = fadeIn(animationName, playTimes, 0.f, 0, "", AnimationFadeOutMode::All);
    }
    else if (!_lastAnimationState)
    {
        animationState = fadeIn(_armature->getArmatureData().getDefaultAnimation()->name, -1, 0.f, 0, "", AnimationFadeOutMode::All);
    }
    else if (!_isPlaying)
    {
        _isPlaying = true;
    }
    else
    {
        animationState = fadeIn(_lastAnimationState->getName(), -1, 0.f, 0, "", AnimationFadeOutMode::All);
    }

    return animationState;
}

AnimationState* Animation::fadeIn(
    const std::string& animationName, int playTimes, float fadeInTime, 
    int layer, const std::string& group, AnimationFadeOutMode fadeOutMode, 
    bool additiveBlending, bool pauseFadeOut, bool pauseFadeIn
)
{
    const auto clipData = mapFind(_animations, animationName);
    if (!clipData)
    {
        return nullptr;
    }

    _isPlaying = true;

    if (fadeInTime != fadeInTime || fadeInTime < 0.f)
    {
        fadeInTime = clipData->fadeInTime;
    }

    if (playTimes < 0)
    {
        playTimes = clipData->playTimes;
    }

    _fadeOut(fadeInTime, layer, group, fadeOutMode, pauseFadeOut);

    _lastAnimationState = BaseObject::borrowObject<AnimationState>();
    _lastAnimationState->_layer = layer;
    _lastAnimationState->_group = group;
    _lastAnimationState->additiveBlending = additiveBlending;
    _lastAnimationState->_fadeIn(
        _armature, clipData->animation ? clipData->animation : clipData, animationName,
        playTimes, clipData->position, clipData->duration, 1.f / clipData->scale, fadeInTime,
        pauseFadeIn
    );
    _animationStates.push_back(_lastAnimationState);

    if (_animationStates.size() > 1)
    {
        std::sort(_animationStates.begin(), _animationStates.end(), _sortAnimationState);
    }

    for (const auto slot : _armature->getSlots())
    {
        if (slot->inheritAnimation)
        {
            const auto childArmature = slot->getChildArmature();
            if (childArmature)
            {
                childArmature->getAnimation().fadeIn(animationName);
            }
        }
    }

    if (fadeInTime == 0.f)
    {
        _armature->advanceTime(0.f);
    }

    return _lastAnimationState;
}

AnimationState * Animation::gotoAndPlayWithTime(const std::string & animationName, float time, int playTimes)
{
    return nullptr;
}

AnimationState * Animation::gotoAndPlayWithFrame(const std::string & animationName, unsigned frame, int playTimes)
{
    return nullptr;
}

AnimationState * Animation::gotoAndPlayWithProgress(const std::string & animationName, float progress, int playTimes)
{
    return nullptr;
}

AnimationState * Animation::gotoAndStopWithTime(const std::string & animationName, float time)
{
    return nullptr;
}

AnimationState * Animation::gotoAndStopWithFrame(const std::string & animationName, unsigned frame)
{
    return nullptr;
}

AnimationState * Animation::gotoAndStopWithProgress(const std::string & animationName, float progress)
{
    return nullptr;
}

bool Animation::hasAnimation(const std::string & animationName)
{
    return _animations.find(animationName) != _animations.end();
}

AnimationState * Animation::getState(const std::string & animationName)
{
    for (std::size_t i = 0, l = _animationStates.size(); i < l; ++i)
    {
        const auto animationState = _animationStates[i];
        if (animationState->getName() == animationName)
        {
            return animationState;
        }
    }

    return nullptr;
}

bool Animation::getIsPlaying() const
{
    return _isPlaying;
}

bool Animation::getIsCompleted() const
{
    if (_lastAnimationState)
    {
        if (!_lastAnimationState->getIsCompleted())
        {
            return false;
        }

        for (const auto animationState : _animationStates)
        {
            if (!animationState->getIsCompleted())
            {
                return false;
            }
        }
    }

    return true;
}

const std::string& Animation::lastAnimationName() const
{
    static const auto DEFAULT_NAME = "";
    return _lastAnimationState ? _lastAnimationState->getName() : DEFAULT_NAME;
}

AnimationState * Animation::lastAnimationState() const
{
    return _lastAnimationState;
}

const std::map<std::string, AnimationData*>& Animation::getAnimations() const
{
    return _animations;
}

void Animation::setAnimations(const std::map<std::string, AnimationData*>& value)
{
    if (_animations == value)
    {
        return;
    }

    _animations.clear();
    _animationNames.clear();

    for (const auto& pair : value)
    {
        _animations[pair.first] = pair.second;
        _animationNames.push_back(pair.first);
    }
}

NAMESPACE_DRAGONBONES_END