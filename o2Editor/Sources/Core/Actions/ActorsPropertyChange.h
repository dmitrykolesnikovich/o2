#pragma once

#include "Core/Actions/IAction.h"

using namespace o2;

namespace o2
{
	class Actor;
}

namespace Editor
{
	class ActorsPropertyChangeAction: public IAction
	{
	public:
		Vector<UInt64>   actorsIds;
		const Type*      componentType;
		String           propertyPath;
		Vector<DataNode> beforeValues;
		Vector<DataNode> afterValues;

	public:
		ActorsPropertyChangeAction();
		ActorsPropertyChangeAction(const Vector<Actor*>& actors, const Type* componentType,
								   const String& propertyPath, const Vector<DataNode>& beforeValues,
								   const Vector<DataNode>& afterValues);

		String GetName() const;
		void Redo();
		void Undo();

		SERIALIZABLE(ActorsPropertyChangeAction);

	protected:
		void SetProperties(Vector<DataNode>& value);
	};
}
