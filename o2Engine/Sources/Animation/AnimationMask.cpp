#include "AnimationMask.h"

namespace o2
{
	float AnimationMask::GetNodeWeight(const String& node) const
	{
		if (weights.ContainsKey(node))
			return weights.Get(node);

		return 1.0f;
	}
}

CLASS_META(o2::AnimationMask)
{
	BASE_CLASS(o2::ISerializable);

	PUBLIC_FIELD(weights).SERIALIZABLE_ATTRIBUTE();

	PUBLIC_FUNCTION(float, GetNodeWeight, const String&);
}
END_META;
