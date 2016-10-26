#include "Spoiler.h"

#include "Animation\AnimatedFloat.h"
#include "Render\Render.h"

namespace o2
{
	UISpoiler::UISpoiler():
		UIWidget()
	{
		mExpandState = AddState("expand", Animation::EaseInOut(this, &mExpandCoef, 0.0f, 1.0f, 0.2f));
		mExpandState->animation.onUpdate = Function<void(float)>(this, &UISpoiler::UpdateExpanding);
		mExpandState->SetState(false);
		UpdateExpanding(0);
	}

	UISpoiler::UISpoiler(const UISpoiler& other):
		UIWidget(other)
	{
		mExpandState = GetStateObject("expand");
		if (!mExpandState)
			mExpandState = AddState("expand", Animation::EaseInOut(this, &mExpandCoef, 0.0f, 1.0f, 0.2f));

		mExpandState->animation.onUpdate = Function<void(float)>(this, &UISpoiler::UpdateExpanding);
		mExpandState->SetState(false);
		UpdateExpanding(0);
	}

	UISpoiler& UISpoiler::operator=(const UISpoiler& other)
	{
		UIWidget::operator=(other);

		mExpandState = GetStateObject("expand");
		if (!mExpandState)
			mExpandState = AddState("expand", Animation::EaseInOut(this, &mExpandCoef, 0.0f, 1.0f, 0.2f));

		mExpandState->animation.onUpdate = Function<void(float)>(this, &UISpoiler::UpdateExpanding);
		mExpandState->SetState(false);
		UpdateExpanding(0);

		return *this;
	}

	void UISpoiler::Expand()
	{
		SetExpanded(true);
	}

	void UISpoiler::Collapse()
	{
		SetExpanded(false);
	}

	void UISpoiler::SetExpanded(bool expand)
	{
		mTargetHeight = 0.0f;
		for (auto child : mChilds)
			mTargetHeight = Math::Max(mBounds.top - child->mBoundsWithChilds.bottom, mTargetHeight);

		if (mExpandState)
			mExpandState->SetState(expand);
	}

	bool UISpoiler::IsExpanded() const
	{
		return mExpandState ? mExpandState->GetState() : false;
	}

	void UISpoiler::Draw()
	{
		if (mFullyDisabled || mIsClipped)
			return;

		for (auto layer : mDrawingLayers)
			layer->Draw();

		OnDrawn();

		bool clipping = !IsFullyExpanded();
		if (clipping)
			o2Render.EnableScissorTest(mBounds);

		for (auto child : mChilds)
			child->Draw();

		if (clipping)
			o2Render.DisableScissorTest();

		for (auto layer : mTopDrawingLayers)
			layer->Draw();

		DrawDebugFrame();
	}

	void UISpoiler::UpdateExpanding(float dt)
	{
		layout.absBottom = layout.absTop - mTargetHeight*mExpandCoef;
	}

	void UISpoiler::UpdateLayout(bool forcible /*= false*/, bool withChildren /*= true*/)
	{
		if (mFullyDisabled && !forcible)
			return;

		if (CheckIsLayoutDrivenByParent(forcible))
			return;

		RecalculateAbsRect();
		UpdateLayersLayouts();

		if (withChildren)
			UpdateChildrenLayouts(forcible);

		if (IsFullyExpanded())
			ExpandSizeByChilds();
	}

	void UISpoiler::OnChildAdded(UIWidget* child)
	{
		child->layout.mDrivenByParent = true;
	}

	void UISpoiler::OnChildRemoved(UIWidget* child)
	{
		child->layout.mDrivenByParent = false;
	}

	void UISpoiler::ExpandSizeByChilds()
	{
		Vec2F relativePivot(0.5f, 1.0f);

		RectF childrenRect;
		if (mChilds.Count() > 0)
			childrenRect = mChilds[0]->layout.mLocalRect;

		for (auto child : mChilds)
		{
			childrenRect.left   = Math::Min(childrenRect.left, child->layout.mLocalRect.left);
			childrenRect.right  = Math::Max(childrenRect.right, child->layout.mLocalRect.right);
			childrenRect.bottom = Math::Min(childrenRect.bottom, child->layout.mLocalRect.bottom);
			childrenRect.top    = Math::Max(childrenRect.top, child->layout.mLocalRect.top);
		}

		Vec2F szDelta = childrenRect.Size() - mChildsAbsRect.Size();

		szDelta *= Vec2F(Math::Sign(layout.mLocalRect.Width()), Math::Sign(layout.mLocalRect.Height()));

		if (szDelta != Vec2F())
		{
			layout.mOffsetMax += szDelta*(Vec2F::One() - relativePivot);
			layout.mOffsetMin -= szDelta*relativePivot;

			UpdateLayout();
		}
	}

	bool UISpoiler::IsFullyExpanded() const
	{
		if (!mExpandState)
			return true;

		return mExpandState->GetState() && !mExpandState->animation.IsPlaying();
	}	 
}
 
CLASS_META(o2::UISpoiler)
{
	BASE_CLASS(o2::UIWidget);

	PROTECTED_FIELD(mExpandState);
	PROTECTED_FIELD(mExpandCoef);
	PROTECTED_FIELD(mTargetHeight);

	PUBLIC_FUNCTION(void, Expand);
	PUBLIC_FUNCTION(void, Collapse);
	PUBLIC_FUNCTION(void, SetExpanded, bool);
	PUBLIC_FUNCTION(bool, IsExpanded);
	PUBLIC_FUNCTION(void, Draw);
	PROTECTED_FUNCTION(void, OnChildAdded, UIWidget*);
	PROTECTED_FUNCTION(void, OnChildRemoved, UIWidget*);
	PROTECTED_FUNCTION(void, UpdateExpanding, float);
	PROTECTED_FUNCTION(void, UpdateLayout, bool, bool);
	PROTECTED_FUNCTION(void, ExpandSizeByChilds);
	PROTECTED_FUNCTION(bool, IsFullyExpanded);
}
END_META;
 