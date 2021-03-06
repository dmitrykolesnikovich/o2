#pragma once

#include "Assets/ImageAsset.h"
#include "Events/DrawableCursorEventsListener.h"
#include "Events/KeyboardEventsListener.h"
#include "Events/ShortcutKeysListener.h"
#include "UI/ScrollArea.h"
#include "Utils/Delegates.h"
#include "Utils/ShortcutKeys.h"

namespace o2
{
	class Sprite;
	class UIButton;
	class UIVerticalLayout;

	// -----------------
	// Context menu item
	// -----------------
	class UIContextMenuItem: public UIWidget, private ShortcutKeysListener
	{
	public:
		Function<void()>     onClick;   // Click function
		Function<void(bool)> onChecked; // Checked function, calls when check was changed and item is checkable

		// Default constructor
		UIContextMenuItem();

		// Copy-constructor
		UIContextMenuItem(const UIContextMenuItem& other);

		// Destructor
		~UIContextMenuItem();

		// Copy-operator
		UIContextMenuItem& operator=(const UIContextMenuItem& other);

		// Returns sub context menu
		UIContextMenu* GetSubMenu() const;

		// Sets checked icon
		void SetChecked(bool checked);

		// Returns is menu checked
		bool IsChecked() const;

		// Sets item checkable
		void SetCheckable(bool checkable);

		// Returns is menu item can be checked
		bool IsCheckable() const;

		// Sets shortcut keys 
		void SetShortcut(const ShortcutKeys& shortcut);

		SERIALIZABLE(UIContextMenuItem);

	protected:
		UIContextMenu* mSubMenu;           // Context sub menu
		bool           mChecked = false;   // Is menu item checked
		bool           mCheckable = false; // Is menu item can be checked

	protected:
		// It is called when child widget was added
		void OnChildAdded(UIWidget* child);

		// This event calling when shortcut hit and this listener has max priority. Calls click callback
		void OnShortcutPressed();

		friend class UIContextMenu;
	};

	// -----------------------
	// Context menu ui element
	// -----------------------
	class UIContextMenu: public UIScrollArea, public DrawableCursorEventsListener, public KeyboardEventsListener
	{
	public:
		// ---------
		// Menu item
		// ---------
		class Item:public ISerializable
		{
		public:
			WString              text;	    // @SERIALIZABLE
			ShortcutKeys         shortcut;  // @SERIALIZABLE
			ImageAssetRef        icon;	    // @SERIALIZABLE
			Vector<Item>         subItems;  // @SERIALIZABLE
			bool                 checked;   // @SERIALIZABLE
			bool                 checkable; // @SERIALIZABLE
			Function<void()>     onClick;   // On click event	
			Function<void(bool)> onChecked; // On checked event	

			Item();

			Item(const WString& text, Vector<Item> subItems, const ImageAssetRef& icon = ImageAssetRef());

			Item(const WString& text, const Function<void()> onClick, const ImageAssetRef& icon = ImageAssetRef(),
				 const ShortcutKeys& shortcut = ShortcutKeys());

			Item(const WString& text, bool checked, Function<void(bool)> onChecked = Function<void(bool)>());

			~Item();

			static Item Separator();

			bool operator==(const Item& other) const;

			SERIALIZABLE(Item);
		};

	public:
		// Default constructor
		UIContextMenu();

		// Constructor from items
		UIContextMenu(Vector<Item> items);

		// Copy-constructor
		UIContextMenu(const UIContextMenu& other);

		// Destructor
		~UIContextMenu();

		// Copy operator
		UIContextMenu& operator=(const UIContextMenu& other);

		// Updates drawables, states and widget
		void Update(float dt);

		// Draws widget
		void Draw();

		// Show from parent context
		void Show(UIContextMenu* parent, const Vec2F& position = o2Input.GetCursorPos());

		// Shows context
		void Show(const Vec2F& position = o2Input.GetCursorPos());

		// Add item
		UIWidget* AddItem(const Item& item);

		// Adds item by path ("node/sub node/target")
		UIWidget* AddItem(const WString& path, const Function<void()>& clickFunc = Function<void()>(),
						  const ImageAssetRef& icon = ImageAssetRef(), const ShortcutKeys& shortcut = ShortcutKeys());

		// Inserts item at position
		UIWidget* InsertItem(const Item& item, int position);

		// Adds array of items
		void AddItems(Vector<Item> items);

		// Inserts array of items at position
		void InsertItems(Vector<Item> items, int position);

		// Returns item at position
		Item GetItem(int position);

		// Sets item at position
		void SetItem(int position, const Item& item);

		// Returns array of all items
		Vector<Item> GetItems() const;

		// Removes item at position
		void RemoveItem(int position);

		// Removes item by path
		void RemoveItem(const WString& path);

		// Removes all items
		void RemoveAllItems();

		// Sets item at position checked (if it checkable)
		void SetItemChecked(int position, bool checked);

		// Returns is item checked
		bool IsItemChecked(int position) const;

		// Returns items vertical layout
		UIVerticalLayout* GetItemsLayout() const;

		// Returns item sample
		UIContextMenuItem* GetItemSample() const;

		// Returns items separator sample
		UIWidget* GetSeparatorSample() const;

		// Returns selection drawable
		Sprite* GetSelectionDrawable() const;

		// Sets selection drawable layout
		void SetSelectionDrawableLayout(const Layout& layout);

		// Returns selection drawable layout
		Layout GetSelectionDrawableLayout() const;

		// Sets minimal fitting size
		void SetMinFitSize(float size);

		// Sets maximum visible items count
		void SetMaxItemsVisible(int count);

		// Sets context menu items shortcut maximum priority
		void SetItemsMaxPriority();

		// Sets context menu items shortcut minimum priority
		void SetItemsMinPriority();

		// Returns is listener scrollable
		bool IsScrollable() const;

		// Updates layout
		void UpdateLayout(bool forcible = false, bool withChildren = true);

		SERIALIZABLE(UIContextMenu);

	protected:
		static UIContextMenu* mVisibleContextMenu;           // Current visible context menu
		const float           mOpenSubMenuDelay = 0.8f;      // Sub menu opengin delay whe cursor hover it

		float                 mFitSizeMin = 40.0f;           // Minimal fitting size @SERIALIZABLE
		int                   mMaxVisibleItems = 100;        // Maximum visible items @SERIALIZABLE

		UIContextMenu*        mParentContextMenu = nullptr;  // Parent visible context menu
		UIContextMenu*        mChildContextMenu = nullptr;	 // Child visible context menu

		UIVerticalLayout*     mItemsLayout = nullptr;        // Items layout
		UIContextMenuItem*    mItemSample = nullptr;         // Item sample @SERIALIZABLE
		UIWidget*             mSeparatorSample = nullptr;    // Items separator sample @SERIALIZABLE
		Sprite*               mSelectionDrawable = nullptr;  // Selection sprite @SERIALIZABLE
		Layout                mSelectionLayout;              // Selection layout, result selection area depends on selected item @SERIALIZABLE

		RectF                 mCurrentSelectionRect;         // Current selection rectangle (for smoothing)
		RectF                 mTargetSelectionRect;          // Target selection rectangle (over selected item)
		Vec2F                 mLastSelectCheckCursor;        // Last cursor position on selection check

		UIContextMenuItem*    mSelectedItem = nullptr;       // Index of selected item
		float                 mSelectSubContextTime = -1.0f; // Time to appearing selected sub context

		bool                  mShownAtFrame = false;         // Is context was shown at current frame

	protected:
		// Checks widget clipping by area
		void CheckClipping(const RectF& clipArea);

		// Fits size by items
		void FitSize();

		// Fits position in screen
		void FitPosition();

		// Special drawing for contexts
		void SpecialDraw();

		// Creates item widget
		UIContextMenuItem* CreateItem(const Item& item);

		// Sets item widget data and parameters
		void SetupItem(UIContextMenuItem* widget, const Item& item);

		// Returns item info
		Item GetItemDef(int idx) const;

		// It is called when visible was changed
		void OnVisibleChanged();

		// Returns item widget under point and stores index in idxPtr, if not null
		UIContextMenuItem* GetItemUnderPoint(const Vec2F& point);

		// Updates hover
		void UpdateHover(const Vec2F& point);

		// It is called when cursor pressed on this
		void OnCursorPressed(const Input::Cursor& cursor);

		// It is called when cursor stay down during frame
		void OnCursorStillDown(const Input::Cursor& cursor);

		// It is called when cursor released (only when cursor pressed this at previous time)
		void OnCursorReleased(const Input::Cursor& cursor);

		// It is called when cursor pressing was broken (when scrolled scroll area or some other)
		void OnCursorPressBreak(const Input::Cursor& cursor);

		// It is called when cursor moved on this (or moved outside when this was pressed)
		void OnCursorMoved(const Input::Cursor& cursor);

		// It is called when key was released
		void OnKeyPressed(const Input::Key& key);

		// Hides context with his parent
		void HideWithParent();

		// Hides context with his child
		void HideWithChild();

		friend class UIManager;
		friend class UIMenuPanel;
	};
}
