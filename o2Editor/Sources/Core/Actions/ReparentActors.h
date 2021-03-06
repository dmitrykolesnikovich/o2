#pragma once

#include "Core/Actions/IAction.h"
#include "Utils/Containers/Vector.h"
#include "Utils/Math/Basis.h"

using namespace o2;

namespace o2
{
	class Actor;
}

namespace Editor
{
	class ReparentActorsAction: public IAction
	{
	public:
		struct ActorInfo
		{
			UInt64 actorId;
			UInt64 lastParentId;
			UInt64 lastPrevActorId;
			int    actorHierarchyIdx;
			Basis  transform;
		};

		Vector<ActorInfo*> actorsInfos;
		UInt64             newParentId;
		UInt64             newPrevActorId;

	public:
		ReparentActorsAction();
		ReparentActorsAction(const Vector<Actor*>& beginActors);
		~ReparentActorsAction();

		void ActorsReparented(Actor* newParent, Actor* prevActor);

		String GetName() const;
		void Redo();
		void Undo();

		SERIALIZABLE(ReparentActorsAction);
	};
}
