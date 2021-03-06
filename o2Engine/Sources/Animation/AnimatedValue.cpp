#include "AnimatedValue.h"

#include "Animation/Animatable.h"
#include "Utils/Math/Color.h"

namespace o2
{
	void AnimatedValue<RectF>::RegInAnimatable(AnimationState* state, const String& path)
	{
		state->mOwner->RegAnimatedValue<RectF>(this, path, state);
	}

	void AnimatedValue<bool>::RegInAnimatable(AnimationState* state, const String& path)
	{
		state->mOwner->RegAnimatedValue<bool>(this, path, state);
	}

	void AnimatedValue<Color4>::RegInAnimatable(AnimationState* state, const String& path)
	{
		state->mOwner->RegAnimatedValue<Color4>(this, path, state);
	}
}

REG_TYPE(o2::AnimatedValue<bool>);
REG_TYPE(o2::AnimatedValue<bool>::Key);

REG_TYPE(o2::AnimatedValue<o2::Color4>);
REG_TYPE(o2::AnimatedValue<o2::Color4>::Key);

CLASS_META(o2::IAnimatedValue)
{
	BASE_CLASS(o2::IAnimation);

	PUBLIC_FIELD(onKeysChanged);

	PUBLIC_FUNCTION(void, SetTargetDelegate, const Function<void()>&);
	PROTECTED_FUNCTION(void, SetTargetVoid, void*);
	PROTECTED_FUNCTION(void, SetTargetVoid, void*, const Function<void()>&);
	PROTECTED_FUNCTION(void, SetTargetPropertyVoid, void*);
	PROTECTED_FUNCTION(void, RegInAnimatable, AnimationState*, const String&);
	PROTECTED_FUNCTION(void, ForceSetTime, float, float);
}
END_META;
