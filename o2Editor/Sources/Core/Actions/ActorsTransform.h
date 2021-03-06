#pragma once

#include "Core/Actions/IAction.h"
#include "Scene/ActorTransform.h"
#include "Utils/Math/Basis.h"

using namespace o2;

namespace o2
{
	class Actor;
}

namespace Editor
{
	class ActorsTransformAction: public IAction
	{
	public:
		Vector<UInt64>         actorsIds;
		Vector<ActorTransform> beforeTransforms;
		Vector<ActorTransform> doneTransforms;

	public:
		ActorsTransformAction();
		ActorsTransformAction(const Vector<Actor*>& actors, const Vector<ActorTransform>& beforeTransforms);

		String GetName() const;
		void Redo();
		void Undo();

		SERIALIZABLE(ActorsTransformAction);
	};
}
