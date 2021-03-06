#pragma once

#include "Events/DrawableCursorEventsListener.h"
#include "Events/KeyboardEventsListener.h"
#include "UI/Widget.h"
#include "Utils/ShortcutKeys.h"

namespace o2
{
	class Text;
	class Sprite;
	class UIToggle;

	// ------------
	// Toggle group
	// ------------
	class UIToggleGroup
	{
	public:
		typedef Vector<UIToggle*> TogglesVec;
		enum class Type { OnlySingleTrue, VerOneClick, HorOneClick };

	public:
		Function<void(bool)> onPressed;  // Toggle group press event
		Function<void(bool)> onReleased; // Toggle group release event

		// Contructor by type
		UIToggleGroup(Type type);

		// Destructor
		~UIToggleGroup();

		// Adds toggle to group
		void AddToggle(UIToggle* toggle);

		// Removes toggle from group
		void RemoveToggle(UIToggle* toggle);

		// Returns all toggles in group
		const TogglesVec& GetToggles() const;

		// Returns toggled toggles in group
		const TogglesVec& GetToggled() const;

	protected:
		bool       mPressed = false;      // Is group in pressed state
		bool       mPressedValue = false; // Group pressed value
		TogglesVec mToggles;              // All toggles in group
		TogglesVec mToggled;              // Toggled toggles in group
		UIToggle*  mOwner = nullptr;      // Owner toggle
		Type       mType;                 // Toggle group type

	protected:
		// It is called when some toggle was toggled, 
		void OnToggled(UIToggle* toggle);

		friend class UIToggle;
	};

	class UIToggle: public UIWidget, public DrawableCursorEventsListener, public KeyboardEventsListener
	{
	public:
		Property<WString>        caption;        // Caption property. Searches text layer with name "caption" or creates them if he's not exist
		Property<bool>           value;          // Current state value property
		Property<UIToggleGroup*> toggleGroup;    // Toggle group property
		Function<void()>         onClick;        // Click event
		Function<void(bool)>     onToggle;       // Toggle event
		Function<void(bool)>     onToggleByUser; // Toggle by user event 
		ShortcutKeys             shortcut;       // Shortcut keys

		// Default constructor
		UIToggle();

		// Copy-constructor
		UIToggle(const UIToggle& other);

		// Assign operator
		UIToggle& operator=(const UIToggle& other);

		// Destructor
		~UIToggle();

		// Updates drawables, states and widget
		void Update(float dt);

		// Sets caption of button. Searches text layer with name "caption". If can't find this layer, creates them
		void SetCaption(const WString& text);

		// Returns caption text from text layer "caption". Returns no data if layer isn't exist
		WString GetCaption() const;

		// Sets current value
		void SetValue(bool value);

		// Sets value as unknown
		void SetValueUnknown();

		// Is value unknown
		bool IsValueUnknown() const;

		// Returns current value
		bool GetValue() const;

		// Returns is this widget can be selected
		bool IsFocusable() const;

		// Sets toggle group
		void SetToggleGroup(UIToggleGroup* toggleGroup);

		// Returns toggle group
		UIToggleGroup* GetToggleGroup() const;

		SERIALIZABLE(UIToggle);

	protected:
		bool           mValue;        // Current value @SERIALIZABLE
		bool           mValueUnknown; // Is value unknown @SERIALIZABLE
		Text*          mCaptionText;  // Caption layer text
		UIWidgetLayer* mBackLayer;    // Background layer
		UIToggleGroup* mToggleGroup;  // Toggle group

	protected:
		// It is called when cursor pressed on this. Sets state "pressed" to true
		void OnCursorPressed(const Input::Cursor& cursor);

		// It is called when cursor released (only when cursor pressed this at previous time). Sets state "pressed" to false.
		// It is called onClicked if cursor is still above this
		void OnCursorReleased(const Input::Cursor& cursor);

		// It is called when cursor pressing was broken (when scrolled scroll area or some other)
		void OnCursorPressBreak(const Input::Cursor& cursor);

		// It is called when cursor enters this object. Sets state "select" to true
		void OnCursorEnter(const Input::Cursor& cursor);

		// It is called when cursor exits this object. Sets state "select" to false
		void OnCursorExit(const Input::Cursor& cursor);

		// It is called when key was pressed
		void OnKeyPressed(const Input::Key& key);

		// It is called when key was released
		void OnKeyReleased(const Input::Key& key);

		// It is called when layer added and updates drawing sequence
		void OnLayerAdded(UIWidgetLayer* layer);

		// It is called when visible was changed
		void OnVisibleChanged();

		// Initializes properties
		void InitializeProperties();

		friend class UIToggleGroup;
	};
}
