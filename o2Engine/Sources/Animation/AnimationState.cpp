#include "AnimationState.h"

#include "Animation/Animatable.h"

namespace o2
{
	AnimationState::AnimationState():
		weight(1.0f), workWeight(1.0f), mOwner(nullptr)
	{}

	AnimationState::AnimationState(const String& name) :
		name(name), weight(1.0f), workWeight(1.0f), mOwner(nullptr)
	{}
}

CLASS_META(o2::AnimationState)
{
	BASE_CLASS(o2::ISerializable);

	PUBLIC_FIELD(name).SERIALIZABLE_ATTRIBUTE();
	PUBLIC_FIELD(animation).SERIALIZABLE_ATTRIBUTE();
	PUBLIC_FIELD(mask).SERIALIZABLE_ATTRIBUTE();
	PUBLIC_FIELD(weight).SERIALIZABLE_ATTRIBUTE();
	PUBLIC_FIELD(workWeight);
	PROTECTED_FIELD(mOwner);
}
END_META;
