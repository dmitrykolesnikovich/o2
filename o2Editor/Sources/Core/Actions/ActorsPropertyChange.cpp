#include "ActorsPropertyChange.h"

#include "Scene/Actor.h"
#include "Scene/Scene.h"

namespace Editor
{

	ActorsPropertyChangeAction::ActorsPropertyChangeAction()
	{
	}

	ActorsPropertyChangeAction::ActorsPropertyChangeAction(const Vector<Actor*>& actors, 
																	   const Type* componentType,
																	   const String& propertyPath, 
																	   const DataNode& beforeValues, 
																	   const DataNode& afterValues):
		actorsIds(actors.Select<UInt64>([](const Actor* x) { return x->GetID(); })), propertyPath(propertyPath),
		beforeValues(beforeValues), afterValues(afterValues), componentType(componentType)
	{
	}

	String ActorsPropertyChangeAction::GetName() const
	{
		return "Property changed";
	}

	void ActorsPropertyChangeAction::Redo()
	{
		SetProperties(afterValues);
	}

	void ActorsPropertyChangeAction::Undo()
	{
		SetProperties(beforeValues);
	}

	void ActorsPropertyChangeAction::SetProperties(DataNode& value)
	{
		Vector<Actor*> actors = actorsIds.Select<Actor*>([](UInt64 id) { return o2Scene.GetActorByID(id); });

		for (auto actor : actors)
		{
			if (actor)
			{
				FieldInfo* fi = nullptr;
				void* ptr = nullptr;

				if (componentType)
				{
					Component* comp = actor->GetComponent(componentType);
					if (comp)
						ptr = comp->GetType().GetFieldPtr(comp, propertyPath, fi);
				}
				else
				{
					ptr = actor->GetType().GetFieldPtr(actor, propertyPath, fi);
				}

				if (fi && ptr)
					fi->Deserialize(ptr, value);

				actor->OnChanged();
			}
		}
	}
}

CLASS_META(Editor::ActorsPropertyChangeAction)
{
	BASE_CLASS(Editor::IAction);

	PUBLIC_FIELD(actorsIds);
	PUBLIC_FIELD(componentType);
	PUBLIC_FIELD(propertyPath);
	PUBLIC_FIELD(beforeValues);
	PUBLIC_FIELD(afterValues);

	PUBLIC_FUNCTION(String, GetName);
	PUBLIC_FUNCTION(void, Redo);
	PUBLIC_FUNCTION(void, Undo);
	PROTECTED_FUNCTION(void, SetProperties, DataNode&);
}
END_META;
 