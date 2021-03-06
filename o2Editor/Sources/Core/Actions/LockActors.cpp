#include "LockActors.h"

#include "Scene/Actor.h"
#include "SceneWindow/SceneEditScreen.h"

namespace Editor
{
	LockActorsAction::LockActorsAction()
	{}

	LockActorsAction::LockActorsAction(const Vector<Actor*>& actors, bool lock):
		lock(lock)
	{
		actorsIds = actors.Select<UInt64>([](Actor* x) { return x->GetID(); });
	}

	String LockActorsAction::GetName() const
	{
		return lock ? "Lock actors" : "Unlock actors";
	}

	void LockActorsAction::Redo()
	{
		for (auto actorId : actorsIds)
		{
			auto actor = o2Scene.GetActorByID(actorId);
			if (actor)
				actor->SetLocked(lock);
		}
	}

	void LockActorsAction::Undo()
	{
		for (auto actorId : actorsIds)
		{
			auto actor = o2Scene.GetActorByID(actorId);
			if (actor)
				actor->SetLocked(!lock);
		}
	}

}

CLASS_META(Editor::LockActorsAction)
{
	BASE_CLASS(Editor::IAction);

	PUBLIC_FIELD(actorsIds);
	PUBLIC_FIELD(lock);

	PUBLIC_FUNCTION(String, GetName);
	PUBLIC_FUNCTION(void, Redo);
	PUBLIC_FUNCTION(void, Undo);
}
END_META;
