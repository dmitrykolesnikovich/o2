#pragma once

#include "Core/Tools/IEditorTool.h"

using namespace o2;

namespace o2
{
	class Sprite;
	class Actor;
}

namespace Editor
{
	// ---------------------
	// Editor selection tool
	// ---------------------
	class SelectionTool: public IEditTool
	{
	public:
		typedef Vector<Actor*> ActorsVec;

	public:
		// Default constructor
		SelectionTool();

		// Destructor
		~SelectionTool();

		IOBJECT(SelectionTool);

	protected:
		Sprite*   mSelectionSprite;			// Selection frame sprite
		ActorsVec mCurrentSelectingActors;	// Current selecting actors (when cursor pressed, but not released yet)
		ActorsVec mBeforeSelectingActors;	// Before selection actors array
		Vec2F     mPressPoint;				// Press point before selecting
		bool      mSelectingActors = false;	// Is selecting actors now

	protected:
		// Draws tool
		void DrawScene();

		// Draws tool
		void DrawScreen();

		// Updates tool
		void Update(float dt);

		// It is called when tool was enabled
		void OnEnabled();

		// It is called when tool was disabled
		void OnDisabled();

		// It is called when actors selection was changed
		void OnActorsSelectionChanged(Vector<Actor*> actors);

		// It is called when cursor pressed on this
		void OnCursorPressed(const Input::Cursor& cursor);

		// It is called when cursor released (only when cursor pressed this at previous time)
		void OnCursorReleased(const Input::Cursor& cursor);

		// It is called when cursor pressing was broken (when scrolled scroll area or some other)
		void OnCursorPressBreak(const Input::Cursor& cursor);

		// It is called when cursor stay down during frame
		void OnCursorStillDown(const Input::Cursor& cursor);

		// It is called when cursor moved on this (or moved outside when this was pressed)
		void OnCursorMoved(const Input::Cursor& cursor);

		// It is called when key was pressed
		void OnKeyPressed(const Input::Key& key);
	};

}
