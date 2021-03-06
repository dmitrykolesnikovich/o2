#pragma once

#include "Core/Actions/IAction.h"
#include "Utils/Containers/Vector.h"

using namespace o2;

namespace o2
{
	class Actor;
}

namespace Editor
{
	class CreateActorsAction: public IAction
	{
	public:
		DataNode       actorsData;
		Vector<UInt64> actorsIds;
		UInt64         insertParentId;
		UInt64         insertPrevActorId;

	public:
		CreateActorsAction();
		CreateActorsAction(const Vector<Actor*>& actors, Actor* parent, Actor* prevActor);

		String GetName() const;
		void Redo();
		void Undo();

		SERIALIZABLE(CreateActorsAction);
	};

}
