#include "ContextMenu.h"

#include "Assets/ImageAsset.h"
#include "Render/Render.h"
#include "Render/Sprite.h"
#include "Render/Text.h"
#include "UI/Button.h"
#include "UI/VerticalLayout.h"

namespace o2
{
	bool UIContextMenu::Item::operator==(const Item& other) const
	{
		return text == other.text && shortcut == other.shortcut && icon == other.icon;
	}

	UIContextMenu::Item::Item():
		checked(false), checkable(false)
	{}

	UIContextMenu::Item::Item(const WString& text, const Function<void()> onClick, const ImageAssetRef& icon /*= ImageAssetRef()*/,
							  const ShortcutKeys& shortcut /*= ShortcutKeys()*/):
		text(text), onClick(onClick), shortcut(shortcut), icon(icon), checked(false), checkable(false)
	{}

	UIContextMenu::Item::Item(const WString& text, Vector<Item> subItems, const ImageAssetRef& icon /*= ImageAssetRef()*/) :
		text(text), subItems(subItems), icon(icon), checked(false), checkable(false)
	{}

	UIContextMenu::Item::Item(const WString& text, bool checked, Function<void(bool)> onChecked /*= Function<void(bool)>()*/) :
		text(text), checked(checked), onChecked(onChecked), checkable(true)
	{}

	UIContextMenu::Item::~Item()
	{}

	UIContextMenu::Item UIContextMenu::Item::Separator()
	{
		return Item("---", Function<void()>());
	}

	UIContextMenu::UIContextMenu():
		UIScrollArea(), DrawableCursorEventsListener(this)
	{
		mItemSample = mnew UIContextMenuItem();
		mItemSample->layout.minHeight = 20.0f;
		mItemSample->AddLayer("icon", nullptr, Layout(Vec2F(0.0f, 0.5f), Vec2F(0.0f, 0.5f), Vec2F(10, 0), Vec2F(10, 0)));
		mItemSample->AddLayer("subIcon", nullptr, Layout(Vec2F(1.0f, 0.5f), Vec2F(1.0f, 0.5f), Vec2F(-10, 0), Vec2F(-10, 0)));
		mItemSample->AddLayer("caption", nullptr, Layout(Vec2F(0.0f, 0.0f), Vec2F(1.0f, 1.0f), Vec2F(20, 0), Vec2F(0, 0)));
		mItemSample->AddLayer("shortcut", nullptr, Layout(Vec2F(0.0f, 0.0f), Vec2F(1.0f, 1.0f), Vec2F(20, 0), Vec2F(0, 0)));

		mSeparatorSample = mnew UIWidget();
		mSeparatorSample->layout.minHeight = 3.0f;
		mSeparatorSample->layout.height = 3.0f;

		mSelectionDrawable = mnew Sprite();

		mItemsLayout = mnew UIVerticalLayout();
		AddChild(mItemsLayout);

		mItemsLayout->expandHeight  = false;
		mItemsLayout->expandWidth   = true;
		mItemsLayout->baseCorner    = BaseCorner::LeftTop;
		mItemsLayout->fitByChildren = true;
		mItemsLayout->layout        = UIWidgetLayout::BothStretch();

		SetVisibleForcible(false);
	}

	UIContextMenu::UIContextMenu(Vector<Item> items):
		UIContextMenu()
	{
		AddItems(items);
	}

	UIContextMenu::UIContextMenu(const UIContextMenu& other):
		UIScrollArea(other), DrawableCursorEventsListener(this), mMaxVisibleItems(other.mMaxVisibleItems)
	{
		mItemSample        = other.mItemSample->Clone();
		mSeparatorSample   = other.mSeparatorSample->Clone();
		mSelectionDrawable = other.mSelectionDrawable->Clone();
		mSelectionLayout   = other.mSelectionLayout;
		mItemsLayout       = FindChild<UIVerticalLayout>();

		RetargetStatesAnimations();
		UpdateLayout();

		SetVisibleForcible(false);
	}

	UIContextMenu::~UIContextMenu()
	{
		delete mItemSample;
		delete mSelectionDrawable;
		delete mSeparatorSample;
	}

	UIContextMenu& UIContextMenu::operator=(const UIContextMenu& other)
	{
		UIScrollArea::operator=(other);

		delete mItemSample;
		delete mSelectionDrawable;
		delete mSeparatorSample;

		mItemSample        = other.mItemSample->Clone();
		mSeparatorSample   = other.mSeparatorSample->Clone();
		mSelectionDrawable = other.mSelectionDrawable->Clone();
		mSelectionLayout   = other.mSelectionLayout;
		mItemsLayout       = FindChild<UIVerticalLayout>();
		mMaxVisibleItems   = other.mMaxVisibleItems;

		mFitSizeMin = other.mFitSizeMin;

		RetargetStatesAnimations();
		UpdateLayout();

		return *this;
	}

	void UIContextMenu::Update(float dt)
	{
		if (mFullyDisabled)
			return;

		UIScrollArea::Update(dt);

		const float rectLerpCoef = 20.0f;
		if (mCurrentSelectionRect != mTargetSelectionRect)
		{
			mCurrentSelectionRect = Math::Lerp(mCurrentSelectionRect, mTargetSelectionRect, dt*rectLerpCoef);
			mSelectionDrawable->SetRect(mCurrentSelectionRect);
		}

		if (!mChildContextMenu && (o2Input.IsCursorPressed() || Math::Abs(o2Input.GetMouseWheelDelta()) > 0.1f) &&
			!layout.mAbsoluteRect.IsInside(o2Input.GetCursorPos()) && !mShownAtFrame && mVisible)
		{
			HideWithParent();
		}

		if (mSelectSubContextTime >= 0.0f)
		{
			mSelectSubContextTime -= dt;

			if (mSelectSubContextTime < 0.0f)
			{
				if (mChildContextMenu)
					mChildContextMenu->HideWithChild();

				if (mSelectedItem && mSelectedItem->GetSubMenu())
					mSelectedItem->GetSubMenu()->Show(this, mSelectedItem->layout.absRightTop);
			}
		}

		mShownAtFrame = false;
	}

	void UIContextMenu::Draw()
	{}

	void UIContextMenu::Show(UIContextMenu* parent, const Vec2F& position /*= o2Input.GetCursorPos()*/)
	{
		if (parent)
		{
			mParentContextMenu = parent;
			parent->mChildContextMenu = this;
		}
		else mVisibleContextMenu = this;

		layout.mOffsetMin.x = Math::Round(position.x);
		layout.mOffsetMax.y = Math::Round(position.y);

		auto hoverState = state["hover"];
		if (hoverState)
		{
			mSelectionDrawable->SetEnabled(true);
			*hoverState = false;
		}
		else mSelectionDrawable->SetEnabled(false);

		FitSize();
		FitPosition();
		UpdateLayout();
		UIWidget::Show();

		mShownAtFrame = true;
	}

	void UIContextMenu::Show(const Vec2F& position /*= o2Input.GetCursorPos()*/)
	{
		Show(nullptr, position);
	}

	UIWidget* UIContextMenu::AddItem(const Item& item)
	{
		if (item.text == "---")
		{
			UIWidget* newItem = mSeparatorSample->Clone();
			newItem->name = "Separator";
			mItemsLayout->AddChild(newItem);

			return newItem;
		}

		UIContextMenuItem* newItem = CreateItem(item);
		mItemsLayout->AddChild(newItem);
		newItem->onClick = item.onClick;

		FitSize();
		FitPosition();
		UpdateLayout();

		return newItem;
	}

	UIWidget* UIContextMenu::AddItem(const WString& path, const Function<void()>& clickFunc /*= Function<void()>()*/,
									 const ImageAssetRef& icon /*= ImageAssetRef()*/, const ShortcutKeys& shortcut /*= ShortcutKeys()*/)
	{
		UIContextMenu* targetContext = this;
		WString targetPath = path;

		while (true)
		{
			int slashPos = targetPath.Find("/");
			if (slashPos < 0)
				break;

			WString subMenu = targetPath.SubStr(0, slashPos);

			UIWidget* subChild = targetContext->mItemsLayout->mChilds.FindMatch([&](auto x) {
				if (auto text = x->GetLayerDrawable<Text>("caption"))
					return text->text == subMenu;

				return false;
			});

			if (!subChild)
				subChild = AddItem(subMenu);

			UIContextMenu* subContext = subChild->FindChild<UIContextMenu>();
			if (!subContext)
			{
				subContext = Clone();
				subContext->RemoveAllItems();

				subChild->AddChild(subContext);

				if (auto subIconLayer = subChild->GetLayer("subIcon"))
					subIconLayer->transparency = 1.0f;
			}

			targetContext = subContext;
			targetPath = targetPath.SubStr(slashPos + 1);
		}

		return targetContext->AddItem(Item(targetPath, clickFunc, icon, shortcut));
	}

	UIWidget* UIContextMenu::InsertItem(const Item& item, int position)
	{
		if (item.text == "---")
		{
			UIWidget* newItem = mSeparatorSample->Clone();
			newItem->name = "Separator";
			mItemsLayout->AddChild(newItem, position);

			return newItem;
		}

		UIContextMenuItem* newItem = CreateItem(item);
		mItemsLayout->AddChild(newItem, position);

		if (item.subItems.Count() == 0)
			newItem->onClick = item.onClick;

		FitSize();
		FitPosition();
		UpdateLayout();

		return newItem;
	}

	void UIContextMenu::AddItems(Vector<Item> items)
	{
		for (auto item : items)
			AddItem(item);
	}

	void UIContextMenu::InsertItems(Vector<Item> items, int position)
	{
		int i = 0;
		for (auto item : items)
		{
			InsertItem(item, position + i);
			i++;
		}
	}

	UIContextMenu::Item UIContextMenu::GetItem(int position)
	{
		if (position > 0 && position < mItemsLayout->GetChilds().Count())
			return GetItemDef(position);

		return Item();
	}

	void UIContextMenu::SetItem(int position, const Item& item)
	{
		if (position < 0 || position >= mItemsLayout->mChilds.Count())
			return;

		UIContextMenuItem* itemWidget = (UIContextMenuItem*)(mItemsLayout->mChilds[position]);
		SetupItem(itemWidget, item);
	}

	void UIContextMenu::OnVisibleChanged()
	{
		interactable = mResVisible;
	}

	UIContextMenuItem* UIContextMenu::GetItemUnderPoint(const Vec2F& point)
	{
		if (!mItemsLayout)
			return nullptr;

		for (auto child : mItemsLayout->mChilds)
		{
			if (child->layout.mAbsoluteRect.IsInside(point) && child->GetType() == TypeOf(UIContextMenuItem))
				return (UIContextMenuItem*)child;
		}

		return nullptr;
	}

	void UIContextMenu::UpdateHover(const Vec2F& point)
	{
		UIContextMenuItem* itemUnderCursor = GetItemUnderPoint(point);

		if (!itemUnderCursor)
		{
			auto hoverState = state["hover"];
			if (hoverState)
			{
				mSelectionDrawable->SetEnabled(true);
				*hoverState = false;
			}
			else
				mSelectionDrawable->SetEnabled(false);

			mSelectedItem = itemUnderCursor;
		}
		else
		{
			mTargetSelectionRect = mSelectionLayout.Calculate(itemUnderCursor->layout.mAbsoluteRect);

			auto hoverState = state["hover"];
			if (hoverState)
			{
				mSelectionDrawable->SetEnabled(true);
				*hoverState = true;
			}
			else
				mSelectionDrawable->SetEnabled(true);

			if (itemUnderCursor != mSelectedItem)
			{
				mSelectSubContextTime = mOpenSubMenuDelay;
				mSelectedItem = itemUnderCursor;
			}
		}
	}

	void UIContextMenu::OnCursorPressed(const Input::Cursor& cursor)
	{}

	void UIContextMenu::OnCursorStillDown(const Input::Cursor& cursor)
	{}

	void UIContextMenu::OnCursorReleased(const Input::Cursor& cursor)
	{
		UIContextMenuItem* itemUnderCursor = GetItemUnderPoint(cursor.position);

		if (itemUnderCursor)
		{
			itemUnderCursor->onClick();

			if (itemUnderCursor->IsCheckable())
			{
				itemUnderCursor->SetChecked(!itemUnderCursor->IsChecked());
				itemUnderCursor->onChecked(itemUnderCursor->IsChecked());
			}
		}

		if (itemUnderCursor && itemUnderCursor->FindChild<UIContextMenu>() == nullptr)
		{
			HideWithParent();
			HideWithChild();
		}
	}

	void UIContextMenu::OnCursorPressBreak(const Input::Cursor& cursor)
	{
		HideWithParent();
		HideWithChild();
	}

	void UIContextMenu::OnCursorMoved(const Input::Cursor& cursor)
	{
		const float checkDeltaThreshold = 2.0f;
		if ((cursor.position - mLastSelectCheckCursor).Length() < checkDeltaThreshold)
			return;

		mLastSelectCheckCursor = cursor.position;

		UpdateHover(cursor.position);
	}

	void UIContextMenu::OnKeyPressed(const Input::Key& key)
	{
// 		if (mVisibleContextMenu && mVisibleContextMenu->IsVisible() && mVisibleContextMenu != this)
// 			return;
// 
// 		for (auto child : mItemsLayout->mChilds)
// 		{
// 			if (child->GetType() == TypeOf(UIContextMenuItem))
// 			{
// 				auto item = (UIContextMenuItem*)child;
// 
// 				if (item->shortcut.IsPressed())
// 				{
// 					item->onClick();
// 					break;
// 				}
// 			}
// 		}
	}

	void UIContextMenu::HideWithParent()
	{
		Hide();

		if (mParentContextMenu)
			mParentContextMenu->HideWithParent();

		mParentContextMenu = nullptr;
		mChildContextMenu = nullptr;
	}

	void UIContextMenu::HideWithChild()
	{
		Hide();

		if (mChildContextMenu)
			mChildContextMenu->HideWithChild();

		mChildContextMenu = nullptr;
	}

	Vector<UIContextMenu::Item> UIContextMenu::GetItems() const
	{
		Vector<Item> res;
		for (int i = 0; i < mItemsLayout->GetChilds().Count(); i++)
			res.Add(GetItemDef(i));

		return res;
	}

	void UIContextMenu::RemoveItem(int position)
	{
		if (position > 0 && position < mItemsLayout->GetChilds().Count())
			mItemsLayout->RemoveChild(mItemsLayout->GetChilds()[position]);
	}

	void UIContextMenu::RemoveItem(const WString& path)
	{
		UIContextMenu* targetContext = this;
		WString targetPath = path;

		while (true)
		{
			int slashPos = targetPath.Find("/");
			if (slashPos < 0)
				break;

			WString subMenu = targetPath.SubStr(0, slashPos);

			UIWidget* subChild = targetContext->mItemsLayout->mChilds.FindMatch([&](auto x) {
				if (auto text = x->GetLayerDrawable<Text>("caption"))
					return text->text == subMenu;

				return false;
			});

			if (!subChild)
			{
				o2Debug.LogError("Failed to remove context item %s", path);
				return;
			}

			UIContextMenu* subContext = subChild->FindChild<UIContextMenu>();
			if (!subContext)
			{
				o2Debug.LogError("Failed to remove context item %s", path);
				return;
			}

			targetContext = subContext;
			targetPath = targetPath.SubStr(slashPos + 1);
		}

		UIWidget* removingItem = targetContext->mItemsLayout->mChilds.FindMatch([&](auto x) {
			if (auto text = x->GetLayerDrawable<Text>("caption"))
				return text->text == targetPath;

			return false;
		});

		if (!removingItem)
		{
			o2Debug.LogError("Failed to remove context item %s", path);
			return;
		}

		targetContext->mItemsLayout->RemoveChild(removingItem);
	}

	void UIContextMenu::RemoveAllItems()
	{
		mItemsLayout->RemoveAllChilds();
		mSelectedItem = nullptr;
	}

	void UIContextMenu::SetItemChecked(int position, bool checked)
	{
		if (position < 0 || position >= mItemsLayout->mChilds.Count())
			return;

		UIContextMenuItem* item = (UIContextMenuItem*)mItemsLayout->mChilds[position];
		if (item->IsCheckable())
			item->SetChecked(checked);
	}

	bool UIContextMenu::IsItemChecked(int position) const
	{
		if (position < 0 || position >= mItemsLayout->mChilds.Count())
			return false;

		UIContextMenuItem* item = (UIContextMenuItem*)mItemsLayout->mChilds[position];
		return item->IsChecked();
	}

	UIVerticalLayout* UIContextMenu::GetItemsLayout() const
	{
		return mItemsLayout;
	}

	UIContextMenuItem* UIContextMenu::GetItemSample() const
	{
		return mItemSample;
	}

	UIWidget* UIContextMenu::GetSeparatorSample() const
	{
		return mSeparatorSample;
	}

	Sprite* UIContextMenu::GetSelectionDrawable() const
	{
		return mSelectionDrawable;
	}

	void UIContextMenu::SetSelectionDrawableLayout(const Layout& layout)
	{
		mSelectionLayout = layout;
	}

	Layout UIContextMenu::GetSelectionDrawableLayout() const
	{
		return mSelectionLayout;
	}

	void UIContextMenu::SetMinFitSize(float size)
	{
		mFitSizeMin = size;
	}

	void UIContextMenu::SetMaxItemsVisible(int count)
	{
		mMaxVisibleItems = count;
	}

	void UIContextMenu::SetItemsMaxPriority()
	{
		for (auto child : mItemsLayout->mChilds)
		{
			UIContextMenuItem* item = dynamic_cast<UIContextMenuItem*>(child);
			if (item)
				item->SetMaxPriority();
		}
	}

	void UIContextMenu::SetItemsMinPriority()
	{
		for (auto child : mItemsLayout->mChilds)
		{
			UIContextMenuItem* item = dynamic_cast<UIContextMenuItem*>(child);
			if (item)
				item->SetMinPriority();
		}
	}

	bool UIContextMenu::IsScrollable() const
	{
		return true;
	}

	UIContextMenu* UIContextMenu::mVisibleContextMenu = nullptr;

	void UIContextMenu::UpdateLayout(bool forcible /*= false*/, bool withChildren /*= true*/)
	{
		layout.mLocalRect.left   = layout.mOffsetMin.x;
		layout.mLocalRect.right  = layout.mOffsetMax.x;
		layout.mLocalRect.bottom = layout.mOffsetMin.y;
		layout.mLocalRect.top    = layout.mOffsetMax.y;
		layout.mAbsoluteRect     = layout.mLocalRect;

		UpdateLayersLayouts();

		mAbsoluteViewArea = mViewAreaLayout.Calculate(layout.mAbsoluteRect);
		mAbsoluteClipArea = mClipAreaLayout.Calculate(layout.mAbsoluteRect);
		Vec2F roundedScrollPos(-Math::Round(mScrollPos.x), Math::Round(mScrollPos.y));

		mChildsAbsRect = mAbsoluteViewArea + roundedScrollPos;

		if (withChildren)
			UpdateChildrenLayouts(true);

		UpdateScrollParams();

		RectF _mChildsAbsRect = mChildsAbsRect;
		mChildsAbsRect = layout.mAbsoluteRect;

		if (mOwnHorScrollBar)
			mHorScrollBar->UpdateLayout(true);

		if (mOwnVerScrollBar)
			mVerScrollBar->UpdateLayout(true);

		mChildsAbsRect = _mChildsAbsRect;
	}

	void UIContextMenu::CheckClipping(const RectF& clipArea)
	{
		mIsClipped = false;

		Vec2F resolution = o2Render.GetCurrentResolution();
		RectF fullScreenRect(resolution*0.5f, resolution*(-0.5f));
		for (auto child : mChilds)
			child->CheckClipping(fullScreenRect);
	}

	void UIContextMenu::FitSize()
	{
		Vec2F size;
		float maxCaption = 0.0f;
		float maxShortcut = 0.0f;

		int i = 0;
		for (auto child : mItemsLayout->GetChilds())
		{
			if (auto childCaption = child->GetLayerDrawable<Text>("caption"))
				maxCaption = Math::Max(childCaption->GetRealSize().x, maxCaption);

			if (auto shortcutCaption = child->GetLayerDrawable<Text>("shortcut"))
				maxShortcut = Math::Max(shortcutCaption->GetRealSize().x, maxShortcut);

			size.y += child->layout.minHeight;

			i++;
			if (i == mMaxVisibleItems)
				break;
		}

		size.x = mFitSizeMin + maxCaption + maxShortcut;
		size.y += layout.height - mAbsoluteViewArea.Height();

		size.x = Math::Min(size.x, (float)o2Render.resolution->x);
		size.y = Math::Min(size.y, (float)o2Render.resolution->y);

		layout.mOffsetMax.x = layout.mOffsetMin.x + size.x;
		layout.mOffsetMin.y = layout.mOffsetMax.y - size.y;
	}

	void UIContextMenu::FitPosition()
	{
		RectF thisRect(layout.mOffsetMin, layout.mOffsetMax);
		RectF screenRect(-o2Render.resolution->x*0.5f, o2Render.resolution->y*0.5f,
						 o2Render.resolution->x*0.5f, -o2Render.resolution->y*0.5f);

		Vec2F offs;

		if (thisRect.left < screenRect.left)
			offs.x = screenRect.left - thisRect.left;

		if (thisRect.right > screenRect.right)
			offs.x = screenRect.right - thisRect.right;

		if (thisRect.top > screenRect.top)
			offs.y = screenRect.top - thisRect.top;

		if (thisRect.bottom < screenRect.bottom)
			offs.y = screenRect.bottom - thisRect.bottom;

		offs.x = Math::Round(offs.x);
		offs.y = Math::Round(offs.y);

		layout.mOffsetMax += offs;
		layout.mOffsetMin += offs;
	}

	void UIContextMenu::SpecialDraw()
	{
		if (mFullyDisabled)
			return;

		for (auto layer : mDrawingLayers)
			layer->Draw();

		IDrawable::OnDrawn();

		o2Render.EnableScissorTest(mAbsoluteClipArea);

		for (auto child : mChilds)
			child->Draw();

		mSelectionDrawable->Draw();

		o2Render.DisableScissorTest();

		for (auto layer : mTopDrawingLayers)
			layer->Draw();

		if (mOwnHorScrollBar)
			mHorScrollBar->Draw();

		if (mOwnVerScrollBar)
			mVerScrollBar->Draw();

		DrawDebugFrame();

		if (mChildContextMenu)
			mChildContextMenu->SpecialDraw();
	}

	UIContextMenuItem* UIContextMenu::CreateItem(const Item& item)
	{
		UIContextMenuItem* itemWidget = mItemSample->Clone();
		SetupItem(itemWidget, item);
		return itemWidget;
	}

	void UIContextMenu::SetupItem(UIContextMenuItem* widget, const Item& item)
	{
		widget->name = (WString)"Context Item " + item.text;

		if (auto iconLayer = widget->GetLayer("icon"))
		{
			if (item.icon)
			{
				Vec2F size = item.icon->GetAtlasRect().Size();

				if (size.x > size.y)
				{
					size.y *= size.x / widget->layout.height;
					size.x = widget->layout.height;
				}
				else
				{
					size.x *= size.y / widget->layout.height;
					size.y = widget->layout.height;
				}

				iconLayer->AddChildLayer("sprite", mnew Sprite(item.icon),
										 Layout(Vec2F(), Vec2F(),
										 Vec2F(-Math::Floor(size.x*0.5f), Math::Floor(size.y*0.5f)),
										 Vec2F(Math::Floor(size.x*0.5f), -Math::Floor(size.y*0.5f))));

				widget->UpdateLayersDrawingSequence();
			}
		}

		if (auto textLayer = widget->GetLayerDrawable<Text>("caption"))
			textLayer->text = item.text;

		if (auto shortcutLayer = widget->GetLayerDrawable<Text>("shortcut"))
			shortcutLayer->text = item.shortcut.AsString();

		if (auto subIconLayer = widget->GetLayer("subIcon"))
			subIconLayer->transparency = item.subItems.Count() > 0 ? 1.0f : 0.0f;

		if (auto checkLayer = widget->GetLayerDrawable<Sprite>("check"))
			checkLayer->enabled = item.checked;

		widget->SetShortcut(item.shortcut);
		widget->SetCheckable(item.checkable);
		widget->onChecked = item.onChecked;

		widget->RemoveAllChilds();
		if (item.subItems.Count() > 0)
		{
			UIContextMenu* subMenu = mnew UIContextMenu(*this);
			subMenu->RemoveAllItems();
			subMenu->AddItems(item.subItems);

			widget->AddChild(subMenu);
		}
	}

	UIContextMenu::Item UIContextMenu::GetItemDef(int idx) const
	{
		Item res;
		auto item = mItemsLayout->mChilds[idx];

		if (item->name == "Separator")
		{
			res.text = "---";
		}
		else
		{
			auto contextItem = (UIContextMenuItem*)item;
			if (auto iconLayer = contextItem->GetLayerDrawable<Sprite>("icon"))
				res.icon = iconLayer->GetImageAsset();

			if (auto textLayer = contextItem->GetLayerDrawable<Text>("caption"))
				res.text = textLayer->text;

			if (auto shortcutLayer = contextItem->GetLayerDrawable<Text>("shortcut"))
				res.shortcut = contextItem->GetShortcut();

			if (auto subMenu = contextItem->FindChild<UIContextMenu>())
				res.subItems = subMenu->GetItems();

			res.checked = contextItem->IsChecked();
			res.onClick = contextItem->onClick;
		}

		return res;
	}

	UIContextMenuItem::UIContextMenuItem():
		UIWidget(), mSubMenu(nullptr)
	{
		RetargetStatesAnimations();
	}

	UIContextMenuItem::UIContextMenuItem(const UIContextMenuItem& other):
		UIWidget(other)
	{
		mSubMenu = FindChild<UIContextMenu>();
		if (mSubMenu) mSubMenu->Hide(true);

		RetargetStatesAnimations();
	}

	UIContextMenuItem::~UIContextMenuItem()
	{}

	UIContextMenuItem& UIContextMenuItem::operator=(const UIContextMenuItem& other)
	{
		UIWidget::operator =(other);
		mSubMenu = FindChild<UIContextMenu>();
		if (mSubMenu) mSubMenu->Hide(true);

		return *this;
	}

	UIContextMenu* UIContextMenuItem::GetSubMenu() const
	{
		return mSubMenu;
	}

	void UIContextMenuItem::SetChecked(bool checked)
	{
		if (auto checkLayer = GetLayerDrawable<Sprite>("check"))
			checkLayer->enabled = checked;

		mChecked = checked;
	}

	bool UIContextMenuItem::IsChecked() const
	{
		return mChecked;
	}

	void UIContextMenuItem::SetCheckable(bool checkable)
	{
		mCheckable = checkable;
	}

	bool UIContextMenuItem::IsCheckable() const
	{
		return mCheckable;
	}

	void UIContextMenuItem::SetShortcut(const ShortcutKeys& shortcut)
	{
		ShortcutKeysListener::SetShortcut(shortcut);
	}

	void UIContextMenuItem::OnChildAdded(UIWidget* child)
	{
		if (child->GetType() == TypeOf(UIContextMenu))
			mSubMenu = (UIContextMenu*)child;
	}

	void UIContextMenuItem::OnShortcutPressed()
	{
		onClick();
	}

}

CLASS_META(o2::UIContextMenuItem)
{
	BASE_CLASS(o2::UIWidget);
	BASE_CLASS(o2::ShortcutKeysListener);

	PUBLIC_FIELD(onClick);
	PUBLIC_FIELD(onChecked);
	PROTECTED_FIELD(mSubMenu);
	PROTECTED_FIELD(mChecked);
	PROTECTED_FIELD(mCheckable);

	PUBLIC_FUNCTION(UIContextMenu*, GetSubMenu);
	PUBLIC_FUNCTION(void, SetChecked, bool);
	PUBLIC_FUNCTION(bool, IsChecked);
	PUBLIC_FUNCTION(void, SetCheckable, bool);
	PUBLIC_FUNCTION(bool, IsCheckable);
	PUBLIC_FUNCTION(void, SetShortcut, const ShortcutKeys&);
	PROTECTED_FUNCTION(void, OnChildAdded, UIWidget*);
	PROTECTED_FUNCTION(void, OnShortcutPressed);
}
END_META;

CLASS_META(o2::UIContextMenu)
{
	BASE_CLASS(o2::UIScrollArea);
	BASE_CLASS(o2::DrawableCursorEventsListener);
	BASE_CLASS(o2::KeyboardEventsListener);

	PROTECTED_FIELD(mOpenSubMenuDelay);
	PROTECTED_FIELD(mFitSizeMin).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mMaxVisibleItems).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mParentContextMenu);
	PROTECTED_FIELD(mChildContextMenu);
	PROTECTED_FIELD(mItemsLayout);
	PROTECTED_FIELD(mItemSample).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mSeparatorSample).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mSelectionDrawable).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mSelectionLayout).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mCurrentSelectionRect);
	PROTECTED_FIELD(mTargetSelectionRect);
	PROTECTED_FIELD(mLastSelectCheckCursor);
	PROTECTED_FIELD(mSelectedItem);
	PROTECTED_FIELD(mSelectSubContextTime);
	PROTECTED_FIELD(mShownAtFrame);

	PUBLIC_FUNCTION(void, Update, float);
	PUBLIC_FUNCTION(void, Draw);
	PUBLIC_FUNCTION(void, Show, UIContextMenu*, const Vec2F&);
	PUBLIC_FUNCTION(void, Show, const Vec2F&);
	PUBLIC_FUNCTION(UIWidget*, AddItem, const Item&);
	PUBLIC_FUNCTION(UIWidget*, AddItem, const WString&, const Function<void()>&, const ImageAssetRef&, const ShortcutKeys&);
	PUBLIC_FUNCTION(UIWidget*, InsertItem, const Item&, int);
	PUBLIC_FUNCTION(void, AddItems, Vector<Item>);
	PUBLIC_FUNCTION(void, InsertItems, Vector<Item>, int);
	PUBLIC_FUNCTION(Item, GetItem, int);
	PUBLIC_FUNCTION(void, SetItem, int, const Item&);
	PUBLIC_FUNCTION(Vector<Item>, GetItems);
	PUBLIC_FUNCTION(void, RemoveItem, int);
	PUBLIC_FUNCTION(void, RemoveItem, const WString&);
	PUBLIC_FUNCTION(void, RemoveAllItems);
	PUBLIC_FUNCTION(void, SetItemChecked, int, bool);
	PUBLIC_FUNCTION(bool, IsItemChecked, int);
	PUBLIC_FUNCTION(UIVerticalLayout*, GetItemsLayout);
	PUBLIC_FUNCTION(UIContextMenuItem*, GetItemSample);
	PUBLIC_FUNCTION(UIWidget*, GetSeparatorSample);
	PUBLIC_FUNCTION(Sprite*, GetSelectionDrawable);
	PUBLIC_FUNCTION(void, SetSelectionDrawableLayout, const Layout&);
	PUBLIC_FUNCTION(Layout, GetSelectionDrawableLayout);
	PUBLIC_FUNCTION(void, SetMinFitSize, float);
	PUBLIC_FUNCTION(void, SetMaxItemsVisible, int);
	PUBLIC_FUNCTION(void, SetItemsMaxPriority);
	PUBLIC_FUNCTION(void, SetItemsMinPriority);
	PUBLIC_FUNCTION(bool, IsScrollable);
	PUBLIC_FUNCTION(void, UpdateLayout, bool, bool);
	PROTECTED_FUNCTION(void, CheckClipping, const RectF&);
	PROTECTED_FUNCTION(void, FitSize);
	PROTECTED_FUNCTION(void, FitPosition);
	PROTECTED_FUNCTION(void, SpecialDraw);
	PROTECTED_FUNCTION(UIContextMenuItem*, CreateItem, const Item&);
	PROTECTED_FUNCTION(void, SetupItem, UIContextMenuItem*, const Item&);
	PROTECTED_FUNCTION(Item, GetItemDef, int);
	PROTECTED_FUNCTION(void, OnVisibleChanged);
	PROTECTED_FUNCTION(UIContextMenuItem*, GetItemUnderPoint, const Vec2F&);
	PROTECTED_FUNCTION(void, UpdateHover, const Vec2F&);
	PROTECTED_FUNCTION(void, OnCursorPressed, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorStillDown, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorReleased, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorPressBreak, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorMoved, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnKeyPressed, const Input::Key&);
	PROTECTED_FUNCTION(void, HideWithParent);
	PROTECTED_FUNCTION(void, HideWithChild);
}
END_META;

CLASS_META(o2::UIContextMenu::Item)
{
	BASE_CLASS(o2::ISerializable);

	PUBLIC_FIELD(text).SERIALIZABLE_ATTRIBUTE();
	PUBLIC_FIELD(shortcut).SERIALIZABLE_ATTRIBUTE();
	PUBLIC_FIELD(icon).SERIALIZABLE_ATTRIBUTE();
	PUBLIC_FIELD(subItems).SERIALIZABLE_ATTRIBUTE();
	PUBLIC_FIELD(checked).SERIALIZABLE_ATTRIBUTE();
	PUBLIC_FIELD(checkable).SERIALIZABLE_ATTRIBUTE();
	PUBLIC_FIELD(onClick);
	PUBLIC_FIELD(onChecked);
}
END_META;
