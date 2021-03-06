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
	class EnableActorsAction: public IAction
	{
	public:
		Vector<UInt64> actorsIds;
		bool           enable;

	public:
		EnableActorsAction();
		EnableActorsAction(const Vector<Actor*>& actors, bool enable);

		String GetName() const;
		void Redo();
		void Undo();

		SERIALIZABLE(EnableActorsAction);
	};
}
