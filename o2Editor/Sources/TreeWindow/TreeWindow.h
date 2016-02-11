#pragma once

#include "Core/WindowsSystem/IEditorWindow.h"
#include "Utils/Singleton.h"

namespace o2
{
	class UIToggle;
	class UIEditBox;
	class UITree;
	class UIButtonGroup;
	class UIToggleGroup;
	class UIContextMenu;
	class Actor;
}

using namespace o2;

// Editor actors tree access macros
#define o2EditorTree TreeWindow::Instance()

// ------------------
// Actors tree window
// ------------------
class TreeWindow: public IEditorWindow, public Singleton<TreeWindow>
{
public:
	IOBJECT(TreeWindow);

	// Returns actors tree widget
	UITree* GetActorsTree() const;

	// Expands all parent actor's nodes in actors tree
	void ExpandActorsTreeNode(Actor* targetActor);

protected:
	UIToggle*      mListTreeToggle;			  // TOggle between list and tree views
	UIEditBox*     mSearchEditBox;			  // Search actors edit box
	UITree*        mActorsTree;				  // Main actors tree
	UIToggleGroup* mEnableActorsTogglesGroup; // Enable actors toggles group
	UIToggleGroup* mLockActorsTogglesGroup;	  // Lock actors toggles group
	UIContextMenu* mTreeContextMenu;		  // Context menu

	bool           mInSearch = false;		  // True when searching actors (mSearchEditBox isn't empty)
	Vector<Actor*> mSearchActors;			  // Array of searched actors

protected:
	// Default constructor
	TreeWindow();

	// Copy-constructor
	TreeWindow(const TreeWindow& other);

	// Destructor
	~TreeWindow();

	// Initializes window
	void InitializeWindow();

	// Calls after that all windows was created
	void PostInitializeWindow();

	// Calls when search button pressed
	void OnSearchPressed();

	// Calls when list-tree toggle changed value
	void OnListTreeToggled(bool value);

	// Calls when search field was changed
	void OnSearchEdited(const WString& searchStr);

	// Searches actors
	void SearchActorsRecursive(Actor* actor, const String& searchStr);

	// Returns actor's parent (For tree widget)
	UnknownType* GetActorsParent(UnknownType* obj);

	// Returns actor's children (For tree widget)
	Vector<UnknownType*> GetActorsChildren(UnknownType* parentObj);

	// Initializes tree node by actor (For tree widget)
	void SetupTreeNodeActor(UITreeNode* node, UnknownType* actorObj);

	// Moves actors to new parent and position, when they was dragged in tree (For tree widget)
	void RearrangeActors(Vector<UnknownType*> objects, UnknownType* parentObj, UnknownType* prevObj);

	// Calls when tree node double clicked (For tree widget)
	void OnTreeNodeDblClick(UITreeNode* node);

	// Calls when tree node clicked by right button (For tree widget)
	void OnTreeRBPressed(UITreeNode* node);

	// Calls when enable actors toggle group pressed
	void EnableActorsGroupPressed(bool value);

	// Calls when enable actors toggle group released
	void EnableActorsGroupReleased(bool value);

	// Calls when lock actors toggle group pressed
	void LockActorsGroupPressed(bool value);

	// Calls when lock actors toggle group released
	void LockActorsGroupReleased(bool value);

	// Calls when some actor needs to created and registers actor insert action
	void CreateActor(Actor* newActor);

	// Calls when pressed "Create new" in context menu
	void OnContextCreateNewPressed();

	// Calls when pressed "Create sprite" in context menu
	void OnContextCreateSprite();

	// Calls when pressed "Create button" in context menu
	void OnContextCreateButton();

	// Calls when pressed "Copy" in context menu
	void OnContextCopyPressed();

	// Calls when pressed "Cut" in context menu
	void OnContextCutPressed();

	// Calls when pressed "Paste" in context menu
	void OnContextPastePressed();

	// Calls when pressed "Delete" in context menu
	void OnContextDeletePressed();

	// Calls when pressed "Duplicate" in context menu
	void OnContextDuplicatePressed();

	// Calls when pressed "Expand all" in context menu
	void OnContextExpandAllPressed();

	// Calls when pressed "Collapse all" in context menu
	void OnContextCollapseAllPressed();

	// Calls when pressed "Lock/unlock" in context menu
	void OnContextLockPressed();

	// Calls when pressed "Enable/disable" in context menu
	void OnContextEnablePressed();

	friend class SceneEditWidget;
};
