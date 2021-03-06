#include "SelectActors.h"

#include "Scene/Actor.h"
#include "SceneWindow/SceneEditScreen.h"

namespace Editor
{
	SelectActorsAction::SelectActorsAction()
	{}

	SelectActorsAction::SelectActorsAction(const Vector<Actor*>& selectedActors, const Vector<Actor*>& prevSelectedActors)
	{
		selectedActorsIds = selectedActors.Select<UInt64>([](Actor* actor) { return actor->GetID(); });
		prevSelectedActorsIds = prevSelectedActors.Select<UInt64>([](Actor* actor) { return actor->GetID(); });
	}

	SelectActorsAction::~SelectActorsAction()
	{}

	String SelectActorsAction::GetName() const
	{
		return "Actors selection";
	}

	void SelectActorsAction::Redo()
	{
		auto& selScreen = o2EditorSceneScreen;

		selScreen.mSelectedActors = selectedActorsIds.Select<Actor*>([&](UInt64 id) { return o2Scene.GetActorByID(id); });
		selScreen.UpdateTopSelectedActors();
		selScreen.OnActorsSelectedFromThis();
		selScreen.mNeedRedraw = true;
	}

	void SelectActorsAction::Undo()
	{
		auto& selScreen = o2EditorSceneScreen;

		selScreen.mSelectedActors = prevSelectedActorsIds.Select<Actor*>([&](UInt64 id) { return o2Scene.GetActorByID(id); });
		selScreen.UpdateTopSelectedActors();
		selScreen.OnActorsSelectedFromThis();
		selScreen.mNeedRedraw = true;
	}

}

CLASS_META(Editor::SelectActorsAction)
{
	BASE_CLASS(Editor::IAction);

	PUBLIC_FIELD(selectedActorsIds);
	PUBLIC_FIELD(prevSelectedActorsIds);

	PUBLIC_FUNCTION(String, GetName);
	PUBLIC_FUNCTION(void, Redo);
	PUBLIC_FUNCTION(void, Undo);
}
END_META;
