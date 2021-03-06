#include "UI/VerticalProgress.h"

namespace o2
{
	UIVerticalProgress::UIVerticalProgress():
		UIWidget(), DrawableCursorEventsListener(this)
	{
		InitializeProperties();
	}

	UIVerticalProgress::UIVerticalProgress(const UIVerticalProgress& other):
		UIWidget(other), DrawableCursorEventsListener(this), mValue(other.mValue), mMinValue(other.mMinValue), 
		mMaxValue(other.mMaxValue), mOrientation(other.mOrientation), mScrollSense(other.mScrollSense)
	{
		mBarLayer = GetLayer("bar");
		mBackLayer = GetLayer("back");

		RetargetStatesAnimations();
		InitializeProperties();
	}

	UIVerticalProgress::~UIVerticalProgress()
	{}

	UIVerticalProgress& UIVerticalProgress::operator=(const UIVerticalProgress& other)
	{
		UIWidget::operator=(other);

		mValue       = other.mValue;
		mMinValue    = other.mMinValue;
		mMaxValue    = other.mMaxValue;
		mOrientation = other.mOrientation;
		mScrollSense = other.mScrollSense;
		mBarLayer    = GetLayer("bar");
		mBackLayer   = GetLayer("back");

		RetargetStatesAnimations();
		UpdateLayout();

		return *this;
	}

	void UIVerticalProgress::Update(float dt)
	{
		if (mFullyDisabled || mIsClipped)
			return;

		UIWidget::Update(dt);

		const float threshold = 0.01f;
		const float smoothCoef = 30.0f;

		if (!Math::Equals(mValue, mSmoothValue, threshold))
		{
			mSmoothValue = Math::Clamp(Math::Lerp(mSmoothValue, mValue, dt*smoothCoef), mMinValue, mMaxValue);

			if (Math::Abs(mValue - mSmoothValue) < threshold)
				mSmoothValue = mValue;

			UpdateProgressLayersLayouts();
		}
	}

	void UIVerticalProgress::SetValue(float value)
	{
		mValue = Math::Clamp(value, mMinValue, mMaxValue);
		UpdateProgressLayersLayouts();

		onChange(mValue);
	}

	void UIVerticalProgress::SetValueForcible(float value)
	{
		mValue = Math::Clamp(value, mMinValue, mMaxValue);
		mSmoothValue = mValue;
		UpdateProgressLayersLayouts();

		onChange(mValue);
	}

	float UIVerticalProgress::GetValue() const
	{
		return mValue;
	}

	void UIVerticalProgress::SetMinValue(float minValue)
	{
		mMinValue = minValue;
		mValue = Math::Max(mMinValue, mValue);
		UpdateProgressLayersLayouts();

		onChange(mValue);
	}

	float UIVerticalProgress::GetMinValue() const
	{
		return mMinValue;
	}

	void UIVerticalProgress::SetMaxValue(float maxValue)
	{
		mMaxValue = maxValue;
		mValue = Math::Min(mMaxValue, mValue);
		UpdateProgressLayersLayouts();

		onChange(value);
	}

	float UIVerticalProgress::GetMaxValue() const
	{
		return mMaxValue;
	}

	void UIVerticalProgress::SetValueRange(float minValue, float maxValue)
	{
		mMaxValue = maxValue;
		mValue = Math::Clamp(mValue, mMinValue, mMaxValue);
		UpdateProgressLayersLayouts();

		onChange(value);
	}

	void UIVerticalProgress::SetScrollSense(float coef)
	{
		mScrollSense = coef;
	}

	float UIVerticalProgress::GetScrollSense() const
	{
		return mScrollSense;
	}

	void UIVerticalProgress::SetOrientation(Orientation orientation)
	{
		mOrientation = orientation;
		UpdateLayersLayouts();
	}

	UIVerticalProgress::Orientation UIVerticalProgress::GetOrientation() const
	{
		return mOrientation;
	}

	bool UIVerticalProgress::IsUnderPoint(const Vec2F& point)
	{
		if (mBackLayer)
			return mDrawingScissorRect.IsInside(point) && mBackLayer->IsUnderPoint(point);

		return false;
	}

	bool UIVerticalProgress::IsScrollable() const
	{
		return true;
	}

	void UIVerticalProgress::OnCursorPressed(const Input::Cursor& cursor)
	{
		auto pressedState = state["pressed"];
		if (pressedState)
			*pressedState = true;

		GetValueFromCursor(cursor);
	}

	void UIVerticalProgress::OnCursorReleased(const Input::Cursor& cursor)
	{
		auto pressedState = state["pressed"];
		if (pressedState)
			*pressedState = false;
	}

	void UIVerticalProgress::OnCursorPressBreak(const Input::Cursor& cursor)
	{
		auto pressedState = state["pressed"];
		if (pressedState)
			*pressedState = false;
	}

	void UIVerticalProgress::OnCursorStillDown(const Input::Cursor& cursor)
	{
		GetValueFromCursor(cursor);
	}

	void UIVerticalProgress::GetValueFromCursor(const Input::Cursor &cursor)
	{
		float height = layout.mAbsoluteRect.Height();
		float d = mMaxValue - mMinValue;
		if (mOrientation == UIVerticalProgress::Orientation::Up)

			SetValue((cursor.position.y - layout.mAbsoluteRect.bottom)/height*d + mMinValue);
		else
			SetValue((height - (cursor.position.y - layout.mAbsoluteRect.bottom))/height*d + mMinValue);
	}

	void UIVerticalProgress::OnCursorEnter(const Input::Cursor& cursor)
	{
		auto selectState = state["hover"];
		if (selectState)
			*selectState = true;
	}

	void UIVerticalProgress::OnCursorExit(const Input::Cursor& cursor)
	{
		auto selectState = state["hover"];
		if (selectState)
			*selectState = false;
	}

	void UIVerticalProgress::OnScrolled(float scroll)
	{
		SetValue(mValue + scroll*mScrollSense);
	}

	void UIVerticalProgress::OnDeserialized(const DataNode& node)
	{
		mBarLayer = GetLayer("bar");
		mBackLayer = GetLayer("back");

		UIWidget::OnDeserialized(node);
	}

	void UIVerticalProgress::OnVisibleChanged()
	{
		interactable = mResVisible;
	}

	void UIVerticalProgress::UpdateLayout(bool forcible /*= false*/, bool withChildren /*= true*/)
	{
		if (CheckIsLayoutDrivenByParent(forcible))
			return;

		RecalculateAbsRect();
		UpdateProgressLayersLayouts();
		UpdateLayersLayouts();

		if (withChildren)
			UpdateChildrenLayouts();
	}

	void UIVerticalProgress::UpdateProgressLayersLayouts()
	{
		if (mBarLayer)
		{
			mBarLayer->layout.offsetMin = Vec2F();
			mBarLayer->layout.offsetMax = Vec2F();

			if (mOrientation == Orientation::Down)
			{
				mBarLayer->layout.anchorMin = Vec2F(0, 1.0f - (mSmoothValue - mMinValue)/(mMaxValue - mMinValue));
				mBarLayer->layout.anchorMax = Vec2F(1, 1);
			}
			else if (mOrientation == Orientation::Up)
			{
				mBarLayer->layout.anchorMin = Vec2F(0, 0);
				mBarLayer->layout.anchorMax = Vec2F(1, (mSmoothValue - mMinValue)/(mMaxValue - mMinValue));
			}
		}

		if (mBackLayer)
			mBackLayer->layout = Layout::BothStretch();

		UpdateLayersLayouts();
	}

	void UIVerticalProgress::OnLayerAdded(UIWidgetLayer* layer)
	{
		if (layer->name == "back")
			mBackLayer = layer;
		else if (layer->name == "bar")
			mBarLayer = layer;

		UpdateProgressLayersLayouts();
	}

	void UIVerticalProgress::InitializeProperties()
	{
		INITIALIZE_PROPERTY(UIVerticalProgress, value, SetValue, GetValue);
		INITIALIZE_PROPERTY(UIVerticalProgress, minValue, SetMinValue, GetMinValue);
		INITIALIZE_PROPERTY(UIVerticalProgress, maxValue, SetMaxValue, GetMaxValue);
		INITIALIZE_PROPERTY(UIVerticalProgress, scrollSense, SetScrollSense, GetScrollSense);
	}
}

CLASS_META(o2::UIVerticalProgress)
{
	BASE_CLASS(o2::UIWidget);
	BASE_CLASS(o2::DrawableCursorEventsListener);

	PUBLIC_FIELD(value);
	PUBLIC_FIELD(minValue);
	PUBLIC_FIELD(maxValue);
	PUBLIC_FIELD(scrollSense);
	PUBLIC_FIELD(onChange);
	PROTECTED_FIELD(mValue).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mSmoothValue);
	PROTECTED_FIELD(mMinValue).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mMaxValue).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mScrollSense).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mOrientation).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mBarLayer);
	PROTECTED_FIELD(mBackLayer);

	PUBLIC_FUNCTION(void, Update, float);
	PUBLIC_FUNCTION(void, SetValue, float);
	PUBLIC_FUNCTION(void, SetValueForcible, float);
	PUBLIC_FUNCTION(float, GetValue);
	PUBLIC_FUNCTION(void, SetMinValue, float);
	PUBLIC_FUNCTION(float, GetMinValue);
	PUBLIC_FUNCTION(void, SetMaxValue, float);
	PUBLIC_FUNCTION(float, GetMaxValue);
	PUBLIC_FUNCTION(void, SetValueRange, float, float);
	PUBLIC_FUNCTION(void, SetScrollSense, float);
	PUBLIC_FUNCTION(float, GetScrollSense);
	PUBLIC_FUNCTION(void, SetOrientation, Orientation);
	PUBLIC_FUNCTION(Orientation, GetOrientation);
	PUBLIC_FUNCTION(bool, IsUnderPoint, const Vec2F&);
	PUBLIC_FUNCTION(bool, IsScrollable);
	PUBLIC_FUNCTION(void, UpdateLayout, bool, bool);
	PROTECTED_FUNCTION(void, UpdateProgressLayersLayouts);
	PROTECTED_FUNCTION(void, OnLayerAdded, UIWidgetLayer*);
	PROTECTED_FUNCTION(void, GetValueFromCursor, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorPressed, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorReleased, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorPressBreak, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorStillDown, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorEnter, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorExit, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnScrolled, float);
	PROTECTED_FUNCTION(void, OnDeserialized, const DataNode&);
	PROTECTED_FUNCTION(void, OnVisibleChanged);
	PROTECTED_FUNCTION(void, InitializeProperties);
}
END_META;

ENUM_META_(o2::UIVerticalProgress::Orientation, Orientation)
{
	ENUM_ENTRY(Down);
	ENUM_ENTRY(Up);
}
END_ENUM_META;
