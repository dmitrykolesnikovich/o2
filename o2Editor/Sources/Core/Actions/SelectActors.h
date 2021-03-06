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
	class SelectActorsAction: public IAction
	{
	public:
		Vector<UInt64> selectedActorsIds;
		Vector<UInt64> prevSelectedActorsIds;

	public:
		SelectActorsAction();
		SelectActorsAction(const Vector<Actor*>& selectedActors, const Vector<Actor*>& prevSelectedActors);
		~SelectActorsAction();

		String GetName() const;
		void Redo();
		void Undo();

		SERIALIZABLE(SelectActorsAction);
	};
}
