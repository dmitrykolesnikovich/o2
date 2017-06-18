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
		Vector<UInt64> actorsIds;
		const Type*    componentType;
		String         propertyPath;
		DataNode       beforeValues;
		DataNode       afterValues;

	public:
		ActorsPropertyChangeAction();
		ActorsPropertyChangeAction(const Vector<Actor*>& actors, const Type* componentType, 
										 const String& propertyPath, const DataNode& beforeValues, 
										 const DataNode& afterValues);

		String GetName() const;
		void Redo();
		void Undo();

		SERIALIZABLE(ActorsPropertyChangeAction);

	protected:
		void SetProperties(DataNode& value);
	};
}
 