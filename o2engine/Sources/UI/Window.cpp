#include "Window.h"

#include "Render/Render.h"
#include "Render/Sprite.h"
#include "Render/Text.h"
#include "UI/Button.h"
#include "Utils/Debug.h"

namespace o2
{
	IOBJECT_CPP(UIWindow);

	UIWindow::UIWindow():
		UIScrollArea()
	{
		InitializeHandles();
		InitializeProperties();
	}

	UIWindow::UIWindow(const UIWindow& other):
		UIScrollArea(other), mHeadDragAreaLayout(other.mHeadDragAreaLayout), mTopDragAreaLayout(other.mTopDragAreaLayout),
		mBottomDragAreaLayout(other.mBottomDragAreaLayout), mLeftDragAreaLayout(other.mLeftDragAreaLayout), 
		mRightDragAreaLayout(other.mRightDragAreaLayout), mLeftTopDragAreaLayout(other.mLeftTopDragAreaLayout), 
		mRightTopDragAreaLayout(other.mRightTopDragAreaLayout), mLeftBottomDragAreaLayout(other.mLeftBottomDragAreaLayout), 
		mRightBottomDragAreaLayout(other.mRightBottomDragAreaLayout)
	{
		mIconDrawable = GetLayerDrawable<Sprite>("icon");
		mCaptionDrawable = GetLayerDrawable<Text>("caption");

		for (auto elem : other.mWindowElements)
		{
			auto newElem = elem->Clone();
			newElem->mParent = this;
			mWindowElements.Add(newElem);
		}

		Ptr<UIButton> closeBtn = mWindowElements.FindMatch(
			[](const Ptr<UIWidget>& x) { return x->GetName() == "closeButton" && x->GetTypeId() == UIButton::type->ID(); });

		if (closeBtn)
			closeBtn->onClick += [&]() { Hide(); };

		RetargetStatesAnimations();
		UpdateLayout();

		InitializeHandles();
		InitializeProperties();
	}

	UIWindow::~UIWindow()
	{
		for (auto elem : mWindowElements)
			elem.Release();
	}

	UIWindow& UIWindow::operator=(const UIWindow& other)
	{
		for (auto elem : mWindowElements)
			elem.Release();

		UIScrollArea::operator=(other);

		mIconDrawable = GetLayerDrawable<Sprite>("icon");
		mCaptionDrawable = GetLayerDrawable<Text>("caption");

		for (auto elem : other.mWindowElements)
		{
			auto newElem = elem->Clone();
			newElem->mParent = this;
			mWindowElements.Add(newElem);
		}
		
		Ptr<UIButton> closeBtn = mWindowElements.FindMatch(
			[](const Ptr<UIWidget>& x) { return x->GetName() == "closeButton" && x->GetTypeId() == UIButton::type->ID(); });

		if (closeBtn)
			closeBtn->onClick += [&]() { Hide(); };

		mHeadDragAreaLayout = other.mHeadDragAreaLayout;
		mTopDragAreaLayout = other.mTopDragAreaLayout;
		mBottomDragAreaLayout = other.mBottomDragAreaLayout;
		mLeftDragAreaLayout = other.mLeftDragAreaLayout;
		mRightDragAreaLayout = other.mRightDragAreaLayout;
		mLeftTopDragAreaLayout = other.mLeftTopDragAreaLayout;
		mRightTopDragAreaLayout = other.mRightTopDragAreaLayout;
		mLeftBottomDragAreaLayout = other.mLeftBottomDragAreaLayout;
		mRightBottomDragAreaLayout = other.mRightBottomDragAreaLayout;

		RetargetStatesAnimations();
		UpdateLayout();

		return *this;
	}

	void UIWindow::Update(float dt)
	{
		UIScrollArea::Update(dt);

		for (auto elem : mWindowElements)
			elem->Update(dt);
	}

	void UIWindow::Draw()
	{
		UIScrollArea::Draw();

		mDrawingDepth = o2Render.GetDrawingDepth() + 1.0f;

		for (auto elem : mWindowElements)
			elem->Draw();

// 		int clr = 0;
// 		o2Render.DrawRectFrame(mHeadDragAreaRect, Color4::SomeColor(clr++));
// 		o2Render.DrawRectFrame(mTopDragAreaRect, Color4::SomeColor(clr++));
// 		o2Render.DrawRectFrame(mBottomDragAreaRect, Color4::SomeColor(clr++));
// 		o2Render.DrawRectFrame(mLeftDragAreaRect, Color4::SomeColor(clr++));
// 		o2Render.DrawRectFrame(mRightDragAreaRect, Color4::SomeColor(clr++));
// 		o2Render.DrawRectFrame(mLeftTopDragAreaRect, Color4::SomeColor(clr++));
// 		o2Render.DrawRectFrame(mRightTopDragAreaRect, Color4::SomeColor(clr++));
// 		o2Render.DrawRectFrame(mLeftBottomDragAreaRect, Color4::SomeColor(clr++));
// 		o2Render.DrawRectFrame(mRightBottomDragAreaRect, Color4::SomeColor(clr++));
	}

	Ptr<UIWidget> UIWindow::AddWindowElement(Ptr<UIWidget> widget)
	{
		widget->mParent = this;
		mWindowElements.Add(widget);
		UpdateLayout();

		return widget;
	}

	void UIWindow::RemoveWindowElement(Ptr<UIWidget> widget)
	{
		mWindowElements.Remove(widget);
		widget.Release();
		UpdateLayout();
	}

	void UIWindow::RemoveAllWindowElements()
	{
		for (auto elem : mWindowElements)
			elem.Release();

		UpdateLayout();
	}

	void UIWindow::SetIcon(Ptr<Sprite> icon)
	{
		if (mIconDrawable)
		{
			mIconDrawable.Release();
			mIconDrawable = icon;
		}
	}

	Ptr<Sprite> UIWindow::GetIcon() const
	{
		return mIconDrawable;
	}

	void UIWindow::SetCaption(const WString& caption)
	{
		if (mCaptionDrawable)
		{
			mCaptionDrawable->SetText(caption);
		}
	}

	WString UIWindow::GetCaption() const
	{
		return mCaptionDrawable->GetText();
	}

	void UIWindow::SetDragAreaLayouts(const Layout& head, const Layout& top, const Layout&bottom, const Layout&left, 
									  const Layout& right, const Layout& leftTop, const Layout& rightTop, 
									  const Layout& leftBottom, const Layout& rightBottom)
	{
		mHeadDragAreaLayout = head;
		mTopDragAreaLayout = top;
		mBottomDragAreaLayout = bottom;
		mLeftDragAreaLayout = left;
		mRightDragAreaLayout = right;
		mLeftTopDragAreaLayout = leftTop;
		mRightTopDragAreaLayout = rightTop;
		mLeftBottomDragAreaLayout = leftBottom;
		mRightBottomDragAreaLayout = rightBottom;
	}

	void UIWindow::UpdateLayout(bool forcible /*= false*/)
	{
		UIScrollArea::UpdateLayout(forcible);

		if (!layout.mDrivenByParent && !forcible)
		{
			RectF _mChildsAbsRect = mChildsAbsRect;
			mChildsAbsRect = layout.mAbsoluteRect;

			for (auto elem : mWindowElements)
				elem->UpdateLayout(true);

			mChildsAbsRect = _mChildsAbsRect;

			mHeadDragAreaRect = mHeadDragAreaLayout.Calculate(layout.mAbsoluteRect);
			mTopDragAreaRect = mTopDragAreaLayout.Calculate(layout.mAbsoluteRect);
			mBottomDragAreaRect = mBottomDragAreaLayout.Calculate(layout.mAbsoluteRect);
			mLeftDragAreaRect = mLeftDragAreaLayout.Calculate(layout.mAbsoluteRect);
			mRightDragAreaRect = mRightDragAreaLayout.Calculate(layout.mAbsoluteRect);
			mLeftTopDragAreaRect = mLeftTopDragAreaLayout.Calculate(layout.mAbsoluteRect);
			mRightTopDragAreaRect = mRightTopDragAreaLayout.Calculate(layout.mAbsoluteRect);
			mLeftBottomDragAreaRect = mLeftBottomDragAreaLayout.Calculate(layout.mAbsoluteRect);
			mRightBottomDragAreaRect = mRightBottomDragAreaLayout.Calculate(layout.mAbsoluteRect);
		}
	}

	void UIWindow::UpdateTransparency()
	{
		UIScrollArea::UpdateTransparency();

		for (auto elem : mWindowElements)
			elem->UpdateTransparency();
	}

	void UIWindow::OnLayerAdded(Ptr<UIWidgetLayer> layer)
	{
		if (layer->name == "icon" && layer->drawable && layer->drawable->GetTypeId() == Sprite::type->ID())
			mIconDrawable = layer->drawable.Cast<Sprite>();

		if (layer->name == "caption" && layer->drawable && layer->drawable->GetTypeId() == Text::type->ID())
			mCaptionDrawable = layer->drawable.Cast<Text>();
	}

	void UIWindow::InitializeHandles()
	{
		mHeadDragHandle.getDepth = [&]() { return mDrawingDepth; };
		mHeadDragHandle.isUnderPoint = [&](const Vec2F& point) { return mHeadDragAreaRect.IsInside(point); };
		mHeadDragHandle.onMoved = [&](const Input::Cursor& cursor) { layout.position += cursor.mDelta; };

		mTopDragHandle.getDepth = [&]() { return mDrawingDepth; };
		mTopDragHandle.isUnderPoint = [&](const Vec2F& point) { return mTopDragAreaRect.IsInside(point); };
		mTopDragHandle.onMoved = [&](const Input::Cursor& cursor) { layout.absTop += cursor.mDelta.y; };

		mBottomDragHandle.getDepth = [&]() { return mDrawingDepth; };
		mBottomDragHandle.isUnderPoint = [&](const Vec2F& point) { return mBottomDragAreaRect.IsInside(point); };
		mBottomDragHandle.onMoved = [&](const Input::Cursor& cursor) { layout.absBottom += cursor.mDelta.y; };

		mLeftDragHandle.getDepth = [&]() { return mDrawingDepth; };
		mLeftDragHandle.isUnderPoint = [&](const Vec2F& point) { return mLeftDragAreaRect.IsInside(point); };
		mLeftDragHandle.onMoved = [&](const Input::Cursor& cursor) { layout.absLeft += cursor.mDelta.x; };

		mRightDragHandle.getDepth = [&]() { return mDrawingDepth; };
		mRightDragHandle.isUnderPoint = [&](const Vec2F& point) { return mRightDragAreaRect.IsInside(point); };
		mRightDragHandle.onMoved = [&](const Input::Cursor& cursor) { layout.absRight += cursor.mDelta.x; };

		mLeftTopDragHandle.getDepth = [&]() { return mDrawingDepth; };
		mLeftTopDragHandle.isUnderPoint = [&](const Vec2F& point) { return mLeftTopDragAreaRect.IsInside(point); };
		mLeftTopDragHandle.onMoved = [&](const Input::Cursor& cursor) { layout.absLeftTop += cursor.mDelta; };

		mLeftBottomDragHandle.getDepth = [&]() { return mDrawingDepth; };
		mLeftBottomDragHandle.isUnderPoint = [&](const Vec2F& point) { return mLeftBottomDragAreaRect.IsInside(point); };
		mLeftBottomDragHandle.onMoved = [&](const Input::Cursor& cursor) { layout.absLeftBottom += cursor.mDelta; };

		mRightTopDragHandle.getDepth = [&]() { return mDrawingDepth; };
		mRightTopDragHandle.isUnderPoint = [&](const Vec2F& point) { return mRightTopDragAreaRect.IsInside(point); };
		mRightTopDragHandle.onMoved = [&](const Input::Cursor& cursor) { layout.absRightTop += cursor.mDelta; };

		mRightBottomDragHandle.getDepth = [&]() { return mDrawingDepth; };
		mRightBottomDragHandle.isUnderPoint = [&](const Vec2F& point) { return mRightBottomDragAreaRect.IsInside(point); };
		mRightBottomDragHandle.onMoved = [&](const Input::Cursor& cursor) { layout.absRightBottom += cursor.mDelta; };
	}

	void UIWindow::InitializeProperties()
	{
		INITIALIZE_PROPERTY(UIWindow, icon, SetIcon, GetIcon);
		INITIALIZE_PROPERTY(UIWindow, caption, SetCaption, GetCaption);
	}
}