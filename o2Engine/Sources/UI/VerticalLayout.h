#pragma once

#include "UI/Widget.h"

namespace o2
{
	// ----------------------
	// Vertical layout widget
	// ----------------------
	class UIVerticalLayout: public UIWidget
	{
	public:
		Property<BaseCorner> baseCorner;    // Base corder property
		Property<float>      spacing;       // Space between widgets property
		Property<RectF>      border;        // Border property
		Property<float>      borderLeft;    // Left border property
		Property<float>      borderRight;   // Right border property
		Property<float>      borderTop;     // Top border property
		Property<float>      borderBottom;  // Bottom border property
		Property<bool>       expandWidth;   // Expand children by width property
		Property<bool>       expandHeight;  // Expand children by height property
		Property<bool>       fitByChildren; // Fitting size by children property

		// Default constructor
		UIVerticalLayout();

		// Copy-constructor
		UIVerticalLayout(const UIVerticalLayout& other);

		// Destructor
		~UIVerticalLayout();

		// Copy operator
		UIVerticalLayout& operator=(const UIVerticalLayout& other);

		// Sets base corner
		void SetBaseCorner(BaseCorner baseCorner);

		// Returns base corner
		BaseCorner GetBaseCorner() const;

		// Sets space between widgets
		void SetSpacing(float spacing);

		// Returns space between widgets
		float GetSpacing() const;

		// Sets border
		void SetBorder(const RectF& border);

		// Returns border
		RectF GetBorder() const;

		// Sets left border
		void SetBorderLeft(float value);

		// Returns left border
		float GetBorderLeft() const;

		// Sets right border
		void SetBorderRight(float value);

		// Returns right border
		float GetBorderRight() const;

		// Sets top border
		void SetBorderTop(float value);

		// Returns top border
		float GetBorderTop() const;

		// Sets bottom border
		void SetBorderBottom(float value);

		// Returns bottom border
		float GetBorderBottom() const;

		// Sets expanding by width
		void SetWidthExpand(bool expand);

		// Returns expanding by width
		bool IsWidthExpand() const;

		// Sets expanding by height
		void SetHeightExpand(bool expand);

		// Returns height expand
		bool IsHeightExpand() const;

		// Sets fitting size by children
		void SetFitByChildren(bool fit);

		// Returns fitting by children
		bool IsFittingByChildren() const;

		// Updates layout
		void UpdateLayout(bool forcible = false, bool withChildren = true);

		SERIALIZABLE(UIVerticalLayout);

	protected:
		BaseCorner mBaseCorner = BaseCorner::Top;  // Base corner of widgets arranging @SERIALIZABLE
		float      mSpacing = 0.0f;                // Space between widgets @SERIALIZABLE
		RectF      mBorder;                        // Border @SERIALIZABLE
		bool       mExpandWidth = true;            // Expanding by width @SERIALIZABLE
		bool       mExpandHeight = true;           // Expanding by height @SERIALIZABLE
		bool       mFitByChildren = false;         // Fitting by children @SERIALIZABLE

	protected:
		// Returns layout height
		float GetMinHeightWithChildren() const;

		// It is called when child widget was added
		void OnChildAdded(UIWidget* child);

		// It is called when child widget was removed
		void OnChildRemoved(UIWidget* child);

		// Invokes required function for childs arranging
		void RearrangeChilds();

		// Arranging child from left to right by bottom, middle and top
		void ArrangeFromTopToBottom();

		// Arranging child from right to left by bottom, middle and top
		void ArrangeFromBottomToTop();

		// Arranging child from center by bottom, middle and top
		void ArrangeFromCenter();

		// Expands size by children
		virtual void ExpandSizeByChilds();

		// Calculates children widths by weights and minimal sizes
		Vector<float> CalculateExpandedHeights();

		// Aligns widget by height with base corner
		void AlignWidgetByWidth(UIWidget* child, float heightAnchor);

		// Updates layout's weight and minimal size
		virtual void UpdateLayoutParametres();

		// Initializes properties
		void InitializeProperties();
	};
}
