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
	class LockActorsAction: public IAction
	{
	public:
		Vector<UInt64> actorsIds;
		bool           lock;

	public:
		LockActorsAction();
		LockActorsAction(const Vector<Actor*>& actors, bool lock);

		String GetName() const;
		void Redo();
		void Undo();

		SERIALIZABLE(LockActorsAction);
	};
}
