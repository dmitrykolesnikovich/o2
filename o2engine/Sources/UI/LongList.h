#pragma once

#include "Events/DrawableCursorEventsListener.h"
#include "Render/Sprite.h"
#include "ScrollArea.h"
#include "UI/VerticalLayout.h"

namespace o2
{
	// ---------------------------------------------
	// List view widget with selection and many data
	// ---------------------------------------------
	class UILongList: public UIScrollArea, public DrawableCursorEventsListener
	{
	public:
		typedef Vector<UnknownType*> UnknownsVec;

	public:
		Property<int>                           selectedItemPos;   // Selected item position property								 				   
		Function<void(int)>                     onSelected;        // Select item position event
		Function<int()>                         getItemsCountFunc; // Items count getting function
		Function<UnknownsVec(int, int)>         getItemsRangeFunc; // Items getting in range function
		Function<void(UIWidget*, UnknownType*)> setupItemFunc;     // Setup item widget function

	    // Default constructor
		UILongList();

		// Copy-constructor
		UILongList(const UILongList& other);

		// Destructor
		~UILongList();

		// Copy-operator
		UILongList& operator=(const UILongList& other);

		// Updates drawables, states and widget
		void Update(float dt);

		// Draws widget
		void Draw();

		// Sets item sample widget. WARNING: Removing all old items!
		void SetItemSample(UIWidget* sample);

		// Returns item sample widget
		UIWidget* GetItemSample() const;

		// Selects item at position
		void SelectItemAt(int position);

		// Returns selected item position
		int GetSelectedItemPosition() const;

		// Returns selection drawable
		Sprite* GetSelectionDrawable() const;

		// Returns hover drawable
		Sprite* GetHoverDrawable() const;

		// Sets selection drawable layout (result rectangle will be calculated by item widget absolute rectangle)
		void SetSelectionDrawableLayout(const Layout& layout);

		// Returns selection drawable layout
		Layout GetSelectionDrawableLayout() const;

		// Sets hover drawable layout (result rectangle will be calculated by item widget absolute rectangle)
		void SetHoverDrawableLayout(const Layout& layout);

		// Returns hover drawable layout
		Layout GetHoverDrawableLayout() const;

		// Returns is listener scrollable
		bool IsScrollable() const;

		// Updates items
		void OnItemsUpdated(bool itemsRearranged = false);

		SERIALIZABLE(UILongList);

	protected:
		UIWidget*         mItemSample;            // Item sample widget @SERIALIZABLE
		Sprite*           mSelectionDrawable;     // Selection sprite @SERIALIZABLE
		Sprite*           mHoverDrawable;         // Item hover drawable @SERIALIZABLE
		Layout            mSelectionLayout;       // Selection layout, result selection area depends on selected item @SERIALIZABLE
		Layout            mHoverLayout;           // Hover layout, result selection area depends on selected item @SERIALIZABLE

		int               mMinVisibleItemIdx;     // Visible item with minimal index
		int               mMaxVisibleItemIdx;     // Visible item with maximal index
		int               mSelectedItem;          // Position of current selected item (-1 if no item isn't selected)

		RectF             mCurrentSelectionRect;  // Current selection rectangle (for smoothing)
		RectF             mTargetSelectionRect;   // Target selection rectangle (over selected item)
		RectF             mCurrentHoverRect;      // Current hover rectangle (for smoothing)
		RectF             mTargetHoverRect;       // Target hover rectangle (over selected item)

		Vec2F             mLastHoverCheckCursor;  // Last cursor position on hover check
		Vec2F             mLastSelectCheckCursor; // Last cursor position on selection check

		WidgetsVec        mItemsPool;             // Items pool

		float             mDrawDepth;             // Drawing depth

	protected:
		// Calculates scroll area
		void CalculateScrollArea();

		// Updates mouse control
		void UpdateControls(float dt);

		// Updates layout
		void UpdateLayout(bool forcible = false);

		// Updates visible items
		void UpdateVisibleItems();

		// Calls when cursor pressed on this
		void OnCursorPressed(const Input::Cursor& cursor);

		// Calls when cursor stay down during frame
		void OnCursorStillDown(const Input::Cursor& cursor);

		// Calls when cursor moved on this (or moved outside when this was pressed)
		void OnCursorMoved(const Input::Cursor& cursor);

		// Calls when cursor released (only when cursor pressed this at previous time)
		void OnCursorReleased(const Input::Cursor& cursor);

		// Calls when cursor pressing was broken (when scrolled scroll area or some other)
		void OnCursorPressBreak(const Input::Cursor& cursor);

		// Calls when cursor exits this object
		void OnCursorExit(const Input::Cursor& cursor);

		// Calls when scrolling
		void OnScrolled(float scroll);

		// Returns item widget under point and stores index in idxPtr, if not null
		UIWidget* GetItemUnderPoint(const Vec2F& point, int* idxPtr);

		// Calls when object was deserialized and trying to reattach states animations target
		void OnDeserialized(const DataNode& node);

		// Updates transparency for this and children widgets
		void UpdateTransparency();

		// Updates hover
		void UpdateHover(const Vec2F& point);

		// Updates selection
		void UpdateSelection(int position);

		// Calls when selected item index was changed
		virtual void OnSelectionChanged();

		// Calls when visible was changed
		void OnVisibleChanged();

		// Initializes properties
		void InitializeProperties();

		friend class UIDropDown;
		friend class UICustomDropDown;
	};
}
