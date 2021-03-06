#include "AssetsIconsScroll.h"

#include "Animation/AnimatedFloat.h"
#include "Animation/Animation.h"
#include "Application/Application.h"
#include "Assets/AnimationAsset.h"
#include "Assets/Assets.h"
#include "Assets/DataAsset.h"
#include "Assets/FolderAsset.h"
#include "AssetsWindow.h"
#include "Core/Actions/CreateActors.h"
#include "Core/EditorApplication.h"
#include "PropertiesWindow/PropertiesWindow.h"
#include "Render/Render.h"
#include "Render/Sprite.h"
#include "Scene/Actor.h"
#include "Scene/Components/ImageComponent.h"
#include "TreeWindow/ActorsTree.h"
#include "UI/EditBox.h"
#include "UI/GridLayout.h"
#include "UI/Label.h"
#include "UI/UIManager.h"
#include "UIAssetIcon.h"
#include "Utils/FileSystem/FileSystem.h"

namespace Editor
{

	UIAssetsIconsScrollArea::UIAssetsIconsScrollArea():
		UIScrollArea()
	{
		mGrid = mnew UIGridLayout();
		mGrid->layout = UIWidgetLayout::BothStretch();
		mGrid->baseCorner = BaseCorner::LeftTop;
		mGrid->fitByChildren = true;
		mGrid->cellSize = mAssetIconSize;
		mGrid->border = RectF(5, 5, 5, 5);
		mGrid->spacing = 5;
		mGrid->arrangeAxis = TwoDirection::Horizontal;
		mGrid->arrangeAxisMaxCells = 5;
		onLayoutChanged += THIS_FUNC(UpdateAssetsGridSize);
		AddChild(mGrid);

		mDragIcon = mnew UIAssetIcon();

		mHightlightSprite = mnew Sprite();
		mSelectionSprite  = mnew Sprite();

		mHightlightAnim.SetTarget(mHightlightSprite);
	}

	UIAssetsIconsScrollArea::UIAssetsIconsScrollArea(const UIAssetsIconsScrollArea& other):
		UIScrollArea(other),
		mHightlightSprite(other.mHightlightSprite->Clone()), mHightlightLayout(other.mHightlightLayout),
		mHightlightAnim(other.mHightlightAnim), mSelectionSprite(other.mSelectionSprite->Clone())
	{
		RemoveAllChilds();

		mGrid = mnew UIGridLayout();
		mGrid->layout = UIWidgetLayout::BothStretch();
		mGrid->baseCorner = BaseCorner::LeftTop;
		mGrid->fitByChildren = true;
		mGrid->cellSize = mAssetIconSize;
		mGrid->border = RectF(5, 5, 5, 5);
		mGrid->spacing = 5;
		mGrid->arrangeAxis = TwoDirection::Horizontal;
		mGrid->arrangeAxisMaxCells = 5;
		onLayoutChanged += THIS_FUNC(UpdateAssetsGridSize);
		AddChild(mGrid);

		onLayoutChanged += THIS_FUNC(UpdateAssetsGridSize);

		mDragIcon = o2UI.CreateWidget<UIAssetIcon>();

		mHightlightAnim.SetTarget(mHightlightSprite);

		RetargetStatesAnimations();
		UpdateLayout();
		InitializeContext();
	}

	UIAssetsIconsScrollArea::~UIAssetsIconsScrollArea()
	{
		delete mDragIcon;
		delete mHightlightSprite;
		delete mSelectionSprite;

		for (auto kv : mIconsPool)
			for (auto icon : kv.Value())
				delete icon;
	}

	UIAssetsIconsScrollArea& UIAssetsIconsScrollArea::operator=(const UIAssetsIconsScrollArea& other)
	{
		delete mDragIcon;
		delete mHightlightSprite;
		delete mSelectionSprite;

		UIScrollArea::operator=(other);

		mDragIcon = other.mDragIcon->Clone();

		mGrid = FindChild<UIGridLayout>();
		mContextMenu = FindChild<UIContextMenu>();

		mHightlightLayout = other.mHightlightLayout;
		mHightlightSprite = other.mHightlightSprite->Clone();
		mHightlightAnim.SetTarget(mHightlightSprite);

		mSelectionSprite = other.mSelectionSprite->Clone();

		RetargetStatesAnimations();
		UpdateLayout();

		return *this;
	}

	void UIAssetsIconsScrollArea::Draw()
	{
		if (mFullyDisabled)
			return;

		for (auto layer : mDrawingLayers)
			layer->Draw();

		IDrawable::OnDrawn();
		CursorAreaEventsListener::OnDrawn();

		o2Render.EnableScissorTest(mAbsoluteClipArea);

		for (auto child : mChilds)
			child->Draw();

		if (mSelecting)
			mSelectionSprite->Draw();

		if (mHightlightSprite)
			mHightlightSprite->Draw();

		o2Render.DisableScissorTest();

		for (auto layer : mTopDrawingLayers)
			layer->Draw();

		if (mOwnHorScrollBar)
			mHorScrollBar->Draw();

		if (mOwnVerScrollBar)
			mVerScrollBar->Draw();

		if (mIsDraggingIcons && mInstSceneDragActors.Count() == 0)
			o2UI.DrawWidgetAtTop(mDragIcon);

		if (IsUIDebugEnabled() || o2Input.IsKeyDown(VK_F1))
			DrawDebugFrame();
	}

	void UIAssetsIconsScrollArea::Update(float dt)
	{
		UIScrollArea::Update(dt);

		if (mHightlightAnim.IsPlaying())
		{
			if (mHightlightIcon)
			{
				mHightlightSprite->SetScale(Vec2F(1.0f, 1.0f));
				mHightlightSprite->SetRect(mHightlightLayout.Calculate(mHightlightIcon->layout.absRect));
			}

			mHightlightAnim.Update(dt);

			if (!mHightlightAnim.IsPlaying())
				mHightlightIcon = nullptr;
		}

		if (mDragEnded)
			EndDragging();

		if (mNeedRebuildAssets)
		{
			mNeedRebuildAssets = false;
			o2Assets.RebuildAssets();
		}
	}

	void UIAssetsIconsScrollArea::SetViewingPath(const String& path)
	{
		if (path == mCurrentPath)
			return;

		mCurrentPath = path;
		UpdateAssetsPath();
		ResetScroll();
	}

	String UIAssetsIconsScrollArea::GetViewingPath() const
	{
		return mCurrentPath;
	}

	void UIAssetsIconsScrollArea::UpdateAssetsPath()
	{
		o2Debug.LogWarning("Update assets folder: %s", mCurrentPath);

		DeselectAllAssets();

		auto prevIcons = mGrid->GetChilds().Select<UIAssetIcon*>([](auto x) { return (UIAssetIcon*)x; });
		prevIcons.ForEach([&](auto x) { FreeAssetIconToPool(x); });
		mGrid->RemoveAllChilds(false);

		AssetInfosVec folderAssetsInfos;
		if (mCurrentPath != "")
			folderAssetsInfos = FolderAssetRef(mCurrentPath)->GetContainingAssetsInfos();
		else
			folderAssetsInfos = o2Assets.GetAssetsTree().mRootAssets.Select<AssetInfo>([](auto x) { return (AssetInfo)*x; });

		auto getAssetTypeSortWeight = [](const AssetInfo& asset) {

			if (asset.assetType == &TypeOf(FolderAsset))
				return 1;
			else if (asset.assetType == &TypeOf(ImageAsset))
				return 2;
			else if (asset.assetType == &TypeOf(DataAsset))
				return 5;
			else if (asset.assetType == &TypeOf(AnimationAsset))
				return 4;
			else if (asset.assetType == &TypeOf(ActorAsset))
				return 3;

			return 100;
		};

		folderAssetsInfos.Sort([&](const AssetInfo& a, const AssetInfo& b) {
			int wa = getAssetTypeSortWeight(a), wb = getAssetTypeSortWeight(b);
			if (wa == wb)
				return a.path > b.path;

			return wa < wb;
		});

		Vector<UIWidget*> addingIcons;
		for (auto asset : folderAssetsInfos)
		{
			UIAssetIcon* assetIcon;

			if (asset.assetType == &TypeOf(FolderAsset))
			{
				assetIcon = GetAssetIconFromPool("folder");
				assetIcon->name = "folder";
			}
			else if (asset.assetType == &TypeOf(ImageAsset))
			{
				assetIcon = GetImageAssetIcon(asset);
				assetIcon->name = "image preview";
			}
			else if (asset.assetType == &TypeOf(DataAsset))
			{
				assetIcon = GetAssetIconFromPool("text");
				assetIcon->name = "text";
			}
			else if (asset.assetType == &TypeOf(AnimationAsset))
			{
				assetIcon = GetAssetIconFromPool("animation");
				assetIcon->name = "animation";
			}
			else if (asset.assetType == &TypeOf(ActorAsset))
			{
				assetIcon = GetAssetIconFromPool("prototype");
				assetIcon->name = "prototype";
			}
			else
			{
				assetIcon = GetAssetIconFromPool("standard");
				assetIcon->name = "standard";
			}

			assetIcon->SetAssetInfo(asset);
			assetIcon->SetState("halfHide", mCuttingAssets.ContainsPred([&](auto x) { return x.first == asset.id; }));
			assetIcon->SetSelectionGroup(this);
			assetIcon->mOwner = this;

			addingIcons.Add(assetIcon);
		}

		mGrid->AddChilds(addingIcons);

		UpdateCuttingAssets();
	}

	bool UIAssetsIconsScrollArea::IsFocusable() const
	{
		return true;
	}

	void UIAssetsIconsScrollArea::HightlightAsset(UID id)
	{
		String assetPath = o2Assets.GetAssetPath(id);
		String assetFolder = o2FileSystem.ExtractPathStr(assetPath);

		SetViewingPath(assetFolder);

		UIAssetIcon* icon = (UIAssetIcon*)(mGrid->GetChilds().FindMatch([=](UIWidget* x) {
			return ((UIAssetIcon*)x)->GetAssetInfo().id == id; }));

		if (!icon)
			return;

		SetScroll(Vec2F(0.0f, -icon->layout.absTop + layout.absTop + GetScroll().y));

		mHightlightIcon = icon;
		mHightlightAnim.RewindAndPlay();
	}

	void UIAssetsIconsScrollArea::SelectAsset(UID id, bool scroll /*= true*/)
	{
		bool selectionChanged = false;

		String assetPath = o2Assets.GetAssetPath(id);
		String assetFolder = o2FileSystem.ExtractPathStr(assetPath);

		SetViewingPath(assetFolder);

		UIAssetIcon* icon = (UIAssetIcon*)(mGrid->GetChilds().FindMatch([=](UIWidget* x) {
			return ((UIAssetIcon*)x)->GetAssetInfo().id == id; }));

		if (!icon)
			return;

		if (!mSelectedAssetsIcons.Contains(icon))
		{
			icon->SetSelected(true);
			mSelectedAssetsIcons.Add(icon);

			selectionChanged = true;
		}

		if (scroll)
			SetScroll(Vec2F(0.0f, -icon->layout.absTop + layout.absTop + GetScroll().y));

		if (selectionChanged)
			OnAssetsSelected();
	}

	void UIAssetsIconsScrollArea::DeselectAllAssets()
	{
		for (auto icon : mSelectedAssetsIcons)
			icon->SetSelected(false);

		mSelectedAssetsIcons.Clear();

		OnAssetsSelected();
	}

	Vector<AssetInfo> UIAssetsIconsScrollArea::GetSelectedAssets() const
	{
		return mSelectedAssetsIcons.Select<AssetInfo>([](UIAssetIcon* x) { return x->GetAssetInfo(); });
	}

	void UIAssetsIconsScrollArea::OnAssetsSelected()
	{
		auto lastSelectedPreloadedAssets = mSelectedPreloadedAssets;
		for (auto asset : lastSelectedPreloadedAssets)
		{
			if (!mSelectedAssetsIcons.ContainsPred([&](UIAssetIcon* x) {
				return x->GetAssetInfo().id == (*asset)->GetAssetId(); }))
			{
				mSelectedPreloadedAssets.Remove(asset);
				(*asset)->Save(false);
				delete asset;
			}
		}

		for (auto icon : mSelectedAssetsIcons)
		{
			if (mSelectedPreloadedAssets.ContainsPred([&](const AssetRef* x) { return (*x)->GetAssetId() == icon->GetAssetInfo().id; }))
				continue;

			AssetRef* iconAsset = mnew AssetRef(o2Assets.GetAssetRef(icon->GetAssetInfo().id));
			mSelectedPreloadedAssets.Add(iconAsset);
		}

		onAssetsSelected(mSelectedAssetsIcons.Select<String>([](UIAssetIcon* x) { return x->GetAssetInfo().path; }));

		if (PropertiesWindow::IsSingletonInitialzed())
		{
			mChangePropertiesTargetsFromThis = true;

			Vector<IObject*> targets;

			if (mSelectedPreloadedAssets.All([](AssetRef* x) { return (*x)->GetType() == TypeOf(ActorAsset); }))
			{
				targets = mSelectedPreloadedAssets.Select<IObject*>([](AssetRef* x) {
					ActorAssetRef asset(*x); return asset->GetActor(); });
			}
			else if (mSelectedPreloadedAssets.All([](AssetRef* x) { return (*x)->GetType() == TypeOf(FolderAsset); }))
				targets.Clear();
			else 
				targets = mSelectedPreloadedAssets.Cast<IObject*>();

			if (!targets.IsEmpty())
				o2EditorProperties.SetTargets(targets, THIS_FUNC(CheckPreloadedAssetsSaving));

			mChangePropertiesTargetsFromThis = false;
		}
	}

	void UIAssetsIconsScrollArea::UpdateLayout(bool forcible /*= false*/, bool withChildren /*= true*/)
	{
		Vec2F localPressPoint = mPressedPoint - mChildsAbsRect.LeftBottom();

		UIScrollArea::UpdateLayout(forcible);

		mPressedPoint = localPressPoint + mChildsAbsRect.LeftBottom();

		if (mSelecting)
			UpdateSelection(*o2Input.GetCursor(0));
	}

	void UIAssetsIconsScrollArea::UpdateCuttingAssets()
	{
		for (auto child : mGrid->GetChilds())
		{
			UIAssetIcon* icon = (UIAssetIcon*)child;
			icon->SetState("halfHide", o2EditorAssets.mCuttingAssets.ContainsPred([=](auto x) {
				return x.first == icon->GetAssetInfo().id; }));
		}
	}

	void UIAssetsIconsScrollArea::OnFocused()
	{
		for (auto icon : mGrid->GetChilds())
			icon->SetState("focused", true);

		UIWidget::OnFocused();
	}

	void UIAssetsIconsScrollArea::OnUnfocused()
	{
		for (auto icon : mGrid->GetChilds())
			icon->SetState("focused", false);

		UIWidget::OnFocused();
	}

	void UIAssetsIconsScrollArea::OnCursorPressed(const Input::Cursor& cursor)
	{
		mPressedPoint = cursor.position;
		BeginSelecting();
		o2UI.FocusWidget(this);
		UpdateSelection(cursor);
	}

	void UIAssetsIconsScrollArea::OnCursorStillDown(const Input::Cursor& cursor)
	{
		if (cursor.delta == Vec2F())
			return;

		float beginDragOrSelectThreshold = 5.0f;

		if (mSelecting)
			UpdateSelection(cursor);
	}

	void UIAssetsIconsScrollArea::BeginSelecting()
	{
		mSelecting = true;

		if (!o2Input.IsKeyDown(VK_CONTROL))
			DeselectAllAssets();
	}

	void UIAssetsIconsScrollArea::OnCursorReleased(const Input::Cursor& cursor)
	{
		if (mSelecting)
			CompleteSelecting();
		else
		{
			if (!o2Input.IsKeyDown(VK_CONTROL))
				DeselectAllAssets();

			UIAssetIcon* iconUnderCursor = GetIconUnderPoint(cursor.position);
			if (iconUnderCursor && !mSelectedAssetsIcons.ContainsPred([=](auto x) { return x == iconUnderCursor; }))
			{
				iconUnderCursor->SetState("assetSelection", true);
				iconUnderCursor->SetSelected(true);
				mSelectedAssetsIcons.Add(iconUnderCursor);

				OnAssetsSelected();
			}
		}
	}

	void UIAssetsIconsScrollArea::BeginDragging(UIAssetIcon* icon)
	{
		mIsDraggingIcons = true;
		mDragEnded = false;

		for (auto sel : mSelectedAssetsIcons)
			sel->Hide();

		*mDragIcon = *mSelectedAssetsIcons.Last();
		mDragIcon->DragDropArea::SetInteractable(false);
		mDragIcon->CursorAreaEventsListener::SetInteractable(false);
		mDragOffset = icon->layout.absRect->Center() - o2Input.GetCursorPos();

		if (mSelectedAssetsIcons.Count() > 1)
			mDragIcon->FindChild<UILabel>()->text = (String)mSelectedAssetsIcons.Count() + " items";
	}

	void UIAssetsIconsScrollArea::EndDragging(bool droppedToThis /*= false*/)
	{
		for (auto sel : mSelectedAssetsIcons)
			sel->Show();

		mIsDraggingIcons = false;
	}

	void UIAssetsIconsScrollArea::UpdateDraggingGraphics()
	{
		mDragIcon->SetParent(nullptr);
		mDragIcon->layout.SetRect(RectF(o2Input.GetCursorPos() - mAssetIconSize*0.5f + mDragOffset,
								  o2Input.GetCursorPos() + mAssetIconSize*0.5f + mDragOffset));
	}

	void UIAssetsIconsScrollArea::OnDragEnter(ISelectableDragableObjectsGroup* group)
	{}

	void UIAssetsIconsScrollArea::OnDraggedAbove(ISelectableDragableObjectsGroup* group)
	{}

	void UIAssetsIconsScrollArea::OnDragExit(ISelectableDragableObjectsGroup* group)
	{}

	void UIAssetsIconsScrollArea::OnDropped(ISelectableDragableObjectsGroup* group)
	{
		if (dynamic_cast<UIAssetsIconsScrollArea*>(group))
			OnDroppedFromThis();
		else if (auto actorsTree = dynamic_cast<UIActorsTree*>(group))
			OnDroppedFromActorsTree(actorsTree);
	}

	void UIAssetsIconsScrollArea::OnDroppedFromThis()
	{
		UIAssetIcon* iconUnderCursor = GetIconUnderPoint(o2Input.GetCursorPos());
		if (iconUnderCursor && iconUnderCursor->GetAssetInfo().assetType == &TypeOf(FolderAsset))
		{
			String destPath = iconUnderCursor->GetAssetInfo().path;
			auto assetsInfos = mSelectedAssetsIcons.Select<AssetInfo>([](auto x) { return x->GetAssetInfo(); });
			o2Assets.MoveAssets(assetsInfos, destPath, true);

			DeselectAllAssets();
		}

		for (auto sel : mSelectedAssetsIcons)
			sel->Show();
	}

	void UIAssetsIconsScrollArea::OnDroppedFromActorsTree(UIActorsTree* actorsTree)
	{
		String destPath = mCurrentPath;

		UIAssetIcon* iconUnderCursor = GetIconUnderPoint(o2Input.GetCursorPos());
		if (iconUnderCursor && iconUnderCursor->GetAssetInfo().assetType == &TypeOf(FolderAsset))
			destPath = iconUnderCursor->GetAssetInfo().path;

		Vector<String> newAssets;
		for (auto actor : actorsTree->GetSelectedActors())
		{
			ActorAssetRef newAsset = actor->MakePrototype();
			String path = destPath.IsEmpty() ? newAsset->GetActor()->name + ".proto" : destPath + "/" + 
				          newAsset->GetActor()->name + ".proto";

			String uniquePath = o2Assets.MakeUniqueAssetName(path);
			newAsset->Save(uniquePath, false);

			newAssets.Add(uniquePath);
		}

		o2Assets.RebuildAssets();

		o2EditorAssets.OpenFolder(destPath);
		o2EditorAssets.SelectAssets(newAssets);
	}

	void UIAssetsIconsScrollArea::RegActorsCreationAction()
	{
		auto firstInstActor = mInstSceneDragActors[0];
		auto parent = firstInstActor->GetParent();
		auto parentChilds = parent ? parent->GetChilds() : o2Scene.GetRootActors();
		int idx = parentChilds.Find(firstInstActor);
		auto prevActor = idx > 0 ? parentChilds[idx - 1] : nullptr;

		auto createAction = mnew CreateActorsAction(mInstSceneDragActors, parent, prevActor);

		o2EditorApplication.DoneAction(createAction);
	}

	void UIAssetsIconsScrollArea::InstantiateDraggingAssets()
	{
		for (auto& sel : mSelectedAssetsIcons)
		{
			auto info = sel->GetAssetInfo();
			Actor* actor = InstantiateAsset(info);
			if (actor)
			{
				actor->name = o2FileSystem.GetPathWithoutDirectories(o2FileSystem.GetFileNameWithoutExtension(info.path));
				mInstSceneDragActors.Add(actor);
			}
		}
	}

	void UIAssetsIconsScrollArea::ClearInstantiatedDraggingAssets()
	{
		for (auto actor : mInstSceneDragActors)
			delete actor;

		mInstSceneDragActors.Clear();
	}

	void UIAssetsIconsScrollArea::OnCursorPressBreak(const Input::Cursor& cursor)
	{}

	void UIAssetsIconsScrollArea::UpdateSelection(const Input::Cursor& cursor)
	{
		RectF selectionRect(cursor.position, mPressedPoint);
		mSelectionSprite->SetRect(selectionRect);

		for (auto icon : mCurrentSelectingIcons)
			icon->SetSelected(false);

		mCurrentSelectingIcons.Clear();

		for (auto child : mGrid->GetChilds())
		{
			if (child->layout.GetAbsoluteRect().IsIntersects(selectionRect))
			{
				UIAssetIcon* icon = (UIAssetIcon*)child;
				icon->SetSelected(true);
				mCurrentSelectingIcons.Add(icon);
			}
		}
	}

	void UIAssetsIconsScrollArea::CompleteSelecting()
	{
		mSelectedAssetsIcons.Add(mCurrentSelectingIcons);
		mCurrentSelectingIcons.Clear();
		mSelecting = false;
	}

	void UIAssetsIconsScrollArea::OnCursorMoved(const Input::Cursor& cursor)
	{}

	void UIAssetsIconsScrollArea::OnCursorRightMouseReleased(const Input::Cursor& cursor)
	{
		UIAssetIcon* iconUnderCursor = GetIconUnderPoint(cursor.position);
		if (iconUnderCursor)
		{
			if (!mSelectedAssetsIcons.Contains(iconUnderCursor))
			{
				DeselectAllAssets();
				iconUnderCursor->SetSelected(true);
				mCurrentSelectingIcons.Add(iconUnderCursor);
			}
		}

		o2UI.FocusWidget(this);
		mContextMenu->Show();
	}

	void UIAssetsIconsScrollArea::OnKeyReleased(const Input::Key& key)
	{
		if (mIsFocused && key == VK_BACK)
		{
			if (mCurrentPath.CountOf("/") > 0)
				o2EditorAssets.OpenFolder(o2FileSystem.GetParentPath(mCurrentPath));
			else
				o2EditorAssets.OpenFolder("");
		}
	}

	void UIAssetsIconsScrollArea::InitializeContext()
	{
		mContextMenu = o2UI.CreateWidget<UIContextMenu>();

		mContextMenu->AddItem("Import", [&]() { OnContextImportPressed(); });
		mContextMenu->AddItem("---");
		mContextMenu->AddItem("Open", [&]() { OnContextOpenPressed(); });
		mContextMenu->AddItem("Show in folder", [&]() { OnContextShowInExplorerPressed(); });
		mContextMenu->AddItem("---");
		mContextMenu->AddItem("Create/Folder", [&]() { OnContextCreateFolderPressed(); });
		mContextMenu->AddItem("Create/Prefab", [&]() { OnContextCreatePrefabPressed(); });
		mContextMenu->AddItem("Create/Script", [&]() { OnContextCreateScriptPressed(); });
		mContextMenu->AddItem("Create/Animation", [&]() { OnContextCreateAnimationPressed(); });
		mContextMenu->AddItem("---");
		mContextMenu->AddItem("Copy", [&]() { OnContextCopyPressed(); }, ImageAssetRef(), ShortcutKeys('C', true));
		mContextMenu->AddItem("Cut", [&]() { OnContextCutPressed(); }, ImageAssetRef(), ShortcutKeys('X', true));
		mContextMenu->AddItem("Paste", [&]() { OnContextPastePressed(); }, ImageAssetRef(), ShortcutKeys('V', true));
		mContextMenu->AddItem("Delete", [&]() { OnContextDeletePressed(); }, ImageAssetRef(), ShortcutKeys(VK_DELETE));

		onFocused = [&]() { mContextMenu->SetItemsMaxPriority(); };
		onUnfocused = [&]() { mContextMenu->SetItemsMinPriority(); };

		AddChild(mContextMenu);
	}

	void UIAssetsIconsScrollArea::PrepareIconsPools()
	{
		int poolSize = 10;
		String iconsStyles[] ={ "standard", "folder", "prototype", "prototype preview", "image preview", "text", "animation" };

		for (auto style : iconsStyles)
		{
			mIconsPool.Add(style, Vector<UIAssetIcon*>());

			for (int i = 0; i < poolSize; i++)
			{
				UIAssetIcon* sample = o2UI.CreateWidget<UIAssetIcon>(style);
				mIconsPool[style].Add(sample);
			}
		}
	}

	UIAssetIcon* UIAssetsIconsScrollArea::GetAssetIconFromPool(const String& style)
	{
		if (!mIconsPool.ContainsKey(style))
			mIconsPool.Add(style, Vector<UIAssetIcon*>());

		int poolResizeStep = 10;

		if (mIconsPool[style].Count() == 0)
		{
			for (int i = 0; i < poolResizeStep; i++)
			{
				UIAssetIcon* sample = o2UI.CreateWidget<UIAssetIcon>(style);
				mIconsPool[style].Add(sample);
			}
		}

		UIAssetIcon* sample = mIconsPool[style].PopBack();
		return sample;
	}

	void UIAssetsIconsScrollArea::FreeAssetIconToPool(UIAssetIcon* icon)
	{
		if (mIconsPool.ContainsKey(icon->name))
			mIconsPool[icon->name].Add(icon);
		else
			delete icon;
	}

	UIAssetIcon* UIAssetsIconsScrollArea::GetImageAssetIcon(const AssetInfo& asset)
	{
		UIAssetIcon* assetIcon = GetAssetIconFromPool("image preview");

		auto previewLayer = assetIcon->layer["preview"];
		Sprite* previewSprite = (Sprite*)previewLayer->drawable;

		ImageAssetRef previewSpriteAsset(asset.path);
		float previewMaxSize = 30;

		if (previewSpriteAsset->width > previewSpriteAsset->height)
		{
			float cf = previewSpriteAsset->height / previewSpriteAsset->width;
			previewLayer->layout = Layout::Based(BaseCorner::Center, Vec2F(previewMaxSize, previewMaxSize*cf),
												 Vec2F(0, 10));
		}
		else
		{
			float cf = previewSpriteAsset->width / previewSpriteAsset->height;
			previewLayer->layout = Layout::Based(BaseCorner::Center, Vec2F(previewMaxSize*cf, previewMaxSize),
												 Vec2F(0, 10));
		}

		previewSprite->image = previewSpriteAsset;
		previewSprite->mode = SpriteMode::Default;

		return assetIcon;
	}

	void UIAssetsIconsScrollArea::UpdateAssetsGridSize()
	{
		float availableWidth = layout.GetWidth() - mGrid->GetBorderLeft() - mGrid->GetBorderRight();
		int preCalcCellsCount = Math::Max(1, Math::FloorToInt(availableWidth/mAssetIconSize.x));
		availableWidth -= preCalcCellsCount*mGrid->GetSpacing();

		mGrid->arrangeAxisMaxCells = Math::Max(1, Math::FloorToInt(availableWidth/mAssetIconSize.x));
	}

	void UIAssetsIconsScrollArea::OnIconDblClicked(UIAssetIcon* icon)
	{
		AssetInfo iconAssetInfo = icon->GetAssetInfo();
		auto assetNameLabel = icon->GetChild("nameLabel");
		if (assetNameLabel && assetNameLabel->IsUnderPoint(o2Input.cursorPos))
		{
			icon->SetState("edit", true);

			auto editBox = (UIEditBox*)icon->GetChild("nameEditBox");

			editBox->text = o2FileSystem.GetFileNameWithoutExtension(
				o2FileSystem.GetPathWithoutDirectories(iconAssetInfo.path));

			editBox->SelectAll();
			editBox->UIWidget::Focus();
			editBox->ResetScroll();

			editBox->onChangeCompleted = [=](const WString& text) {
				icon->SetState("edit", false);
				String ext = o2FileSystem.GetFileExtension(iconAssetInfo.path);
				if (ext.IsEmpty())
					o2Assets.RenameAsset(iconAssetInfo, text, false);
				else
					o2Assets.RenameAsset(iconAssetInfo, text + "." + o2FileSystem.GetFileExtension(iconAssetInfo.path), false);
				mNeedRebuildAssets = true;
			};

			return;
		}

		if (iconAssetInfo.assetType == &TypeOf(FolderAsset))
			o2EditorAssets.OpenFolder(iconAssetInfo.path);
		else
			o2EditorAssets.OpenAndEditAsset(iconAssetInfo.id);
	}

	UIAssetIcon* UIAssetsIconsScrollArea::GetIconUnderPoint(const Vec2F& point) const
	{
		for (auto child : mGrid->GetChilds())
		{
			if (child->layout.IsUnderPoint(point))
			{
				UIAssetIcon* icon = (UIAssetIcon*)child;
				return icon;
			}
		}

		return nullptr;
	}

	Sprite* UIAssetsIconsScrollArea::GetHightlightDrawable() const
	{
		return mHightlightSprite;
	}

	void UIAssetsIconsScrollArea::SetHightlightAnimation(const Animation& animation)
	{
		mHightlightAnim = animation;
		mHightlightAnim.SetTarget(mHightlightSprite);
	}

	void UIAssetsIconsScrollArea::SetHightlightLayout(const Layout& layout)
	{
		mHightlightLayout = layout;
	}

	Sprite* UIAssetsIconsScrollArea::GetSelectingDrawable() const
	{
		return mSelectionSprite;
	}

	bool UIAssetsIconsScrollArea::IsUnderPoint(const Vec2F& point)
	{
		return UIWidget::IsUnderPoint(point);
	}

#undef CopyFile

	void UIAssetsIconsScrollArea::OnContextCopyPressed()
	{
		o2EditorAssets.CopyAssets(
			mSelectedAssetsIcons.Select<String>([](UIAssetIcon* x) { return x->GetAssetInfo().path; }));
	}

	void UIAssetsIconsScrollArea::OnContextCutPressed()
	{
		o2EditorAssets.CutAssets(
			mSelectedAssetsIcons.Select<String>([](UIAssetIcon* x) { return x->GetAssetInfo().path; }));
	}

	void UIAssetsIconsScrollArea::OnContextPastePressed()
	{
		o2EditorAssets.PasteAssets(mCurrentPath);
	}


	void UIAssetsIconsScrollArea::OnContextDeletePressed()
	{
		for (auto asset : mSelectedPreloadedAssets)
			delete asset;

		mSelectedPreloadedAssets.Clear();

		o2EditorAssets.DeleteAssets(
			mSelectedAssetsIcons.Select<String>([](UIAssetIcon* x) { return x->GetAssetInfo().path; }));
	}

	void UIAssetsIconsScrollArea::OnContextOpenPressed()
	{
		if (mSelectedAssetsIcons.Count() > 0)
			o2EditorAssets.OpenAndEditAsset(mSelectedAssetsIcons.Last()->GetAssetInfo().id);
	}

	void UIAssetsIconsScrollArea::OnContextShowInExplorerPressed()
	{
		if (mSelectedAssetsIcons.Count() > 0)
			o2EditorAssets.OpenAsset(mSelectedAssetsIcons.Last()->GetAssetInfo().id);
	}

	void UIAssetsIconsScrollArea::OnContextImportPressed()
	{
		o2EditorAssets.ImportAssets(mCurrentPath);
	}

	void UIAssetsIconsScrollArea::OnContextCreateFolderPressed()
	{
		o2EditorAssets.CreateFolderAsset(mCurrentPath);
	}

	void UIAssetsIconsScrollArea::OnContextCreatePrefabPressed()
	{
		o2EditorAssets.CreatePrefabAsset(mCurrentPath);
	}

	void UIAssetsIconsScrollArea::OnContextCreateScriptPressed()
	{
		o2EditorAssets.CreateScriptAsset(mCurrentPath);
	}

	void UIAssetsIconsScrollArea::OnContextCreateAnimationPressed()
	{
		o2EditorAssets.CreateAnimationAsset(mCurrentPath);
	}

	Actor* UIAssetsIconsScrollArea::InstantiateAsset(const ImageAssetRef& asset)
	{
		return mnew Actor({ mnew ImageComponent(asset) });
	}

	Actor* UIAssetsIconsScrollArea::InstantiateAsset(const ActorAssetRef& asset)
	{
		return mnew Actor(*(asset->GetActor()));
	}

	void UIAssetsIconsScrollArea::CheckPreloadedAssetsSaving()
	{
		if (mChangePropertiesTargetsFromThis)
			return;

		for (auto asset : mSelectedPreloadedAssets)
		{
			(*asset)->Save(false);
			delete asset;
		}

		mSelectedPreloadedAssets.Clear();
	}

	void UIAssetsIconsScrollArea::Select(SelectableDragableObject* object, bool sendOnSelectionChanged)
	{
		UIAssetIcon* icon = dynamic_cast<UIAssetIcon*>(object);

		if (!mSelectedAssetsIcons.Contains(icon))
		{
			icon->SetSelected(true);
			mSelectedAssetsIcons.Add(icon);

			if (sendOnSelectionChanged)
				OnAssetsSelected();
		}
	}

	void UIAssetsIconsScrollArea::Deselect(SelectableDragableObject* object)
	{
		UIAssetIcon* icon = dynamic_cast<UIAssetIcon*>(object);

		if (mSelectedAssetsIcons.Contains(icon))
		{
			icon->SetSelected(false);
			mSelectedAssetsIcons.Remove(icon);

			OnAssetsSelected();
		}
	}

	void UIAssetsIconsScrollArea::AddSelectableObject(SelectableDragableObject* object)
	{}

	void UIAssetsIconsScrollArea::RemoveSelectableObject(SelectableDragableObject* object)
	{}

	void UIAssetsIconsScrollArea::OnSelectableObjectCursorReleased(SelectableDragableObject* object, const Input::Cursor& cursor)
	{
		if (!o2Input.IsKeyDown(VK_CONTROL))
			DeselectAllAssets();

		Select(object);
	}

	void UIAssetsIconsScrollArea::OnSelectableObjectBeganDragging(SelectableDragableObject* object)
	{
		if (!o2Input.IsKeyDown(VK_CONTROL) && !mSelectedAssetsIcons.Contains(dynamic_cast<UIAssetIcon*>(object)))
		{
			for (auto icon : mSelectedAssetsIcons)
				icon->SetSelected(false);

			mSelectedAssetsIcons.Clear();
		}

		if (!object->IsSelected())
			Select(object, false);
	}

	Actor* UIAssetsIconsScrollArea::InstantiateAsset(const AssetInfo& assetInfo)
	{
		if (assetInfo.assetType == &TypeOf(ImageAsset))
			return InstantiateAsset(ImageAssetRef(assetInfo.id));
		else if (assetInfo.assetType == &TypeOf(ActorAsset))
			return InstantiateAsset(ActorAssetRef(assetInfo.id));

		return nullptr;
	}

	ISelectableDragableObjectsGroup::SelectDragObjectsVec UIAssetsIconsScrollArea::GetSelectedDragObjects() const
	{
		return mSelectedAssetsIcons.Cast<SelectableDragableObject*>();
	}

	ISelectableDragableObjectsGroup::SelectDragObjectsVec UIAssetsIconsScrollArea::GetAllObjects() const
	{
		return mGrid->GetChilds().Cast<SelectableDragableObject*>();
	}

	void UIAssetsIconsScrollArea::Select(SelectableDragableObject* object)
	{
		Select(object, true);
	}
}

CLASS_META(Editor::UIAssetsIconsScrollArea)
{
	BASE_CLASS(o2::UIScrollArea);
	BASE_CLASS(o2::DragDropArea);
	BASE_CLASS(o2::KeyboardEventsListener);
	BASE_CLASS(o2::ISelectableDragableObjectsGroup);

	PUBLIC_FIELD(onAssetsSelected);
	PROTECTED_FIELD(mAssetIconSize);
	PROTECTED_FIELD(mCurrentPath);
	PROTECTED_FIELD(mGrid);
	PROTECTED_FIELD(mContextMenu);
	PROTECTED_FIELD(mSelectedAssetsIcons);
	PROTECTED_FIELD(mSelectedPreloadedAssets);
	PROTECTED_FIELD(mHightlightIcon);
	PROTECTED_FIELD(mHightlightAnim).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mHightlightSprite).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mHightlightLayout).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mIconsPool);
	PROTECTED_FIELD(mSelectionSprite).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mSelecting);
	PROTECTED_FIELD(mPressedPoint);
	PROTECTED_FIELD(mCurrentSelectingIcons);
	PROTECTED_FIELD(mIsDraggingIcons);
	PROTECTED_FIELD(mDragEnded);
	PROTECTED_FIELD(mDragIcon);
	PROTECTED_FIELD(mDragOffset);
	PROTECTED_FIELD(mInstSceneDragActors);
	PROTECTED_FIELD(mCuttingAssets);
	PROTECTED_FIELD(mNeedRebuildAssets);
	PROTECTED_FIELD(mChangePropertiesTargetsFromThis);

	PUBLIC_FUNCTION(void, Draw);
	PUBLIC_FUNCTION(void, Update, float);
	PUBLIC_FUNCTION(void, SetViewingPath, const String&);
	PUBLIC_FUNCTION(String, GetViewingPath);
	PUBLIC_FUNCTION(void, UpdateAssetsPath);
	PUBLIC_FUNCTION(bool, IsFocusable);
	PUBLIC_FUNCTION(void, HightlightAsset, UID);
	PUBLIC_FUNCTION(void, SelectAsset, UID, bool);
	PUBLIC_FUNCTION(void, DeselectAllAssets);
	PUBLIC_FUNCTION(Vector<AssetInfo>, GetSelectedAssets);
	PUBLIC_FUNCTION(UIAssetIcon*, GetIconUnderPoint, const Vec2F&);
	PUBLIC_FUNCTION(Sprite*, GetHightlightDrawable);
	PUBLIC_FUNCTION(void, SetHightlightAnimation, const Animation&);
	PUBLIC_FUNCTION(void, SetHightlightLayout, const Layout&);
	PUBLIC_FUNCTION(Sprite*, GetSelectingDrawable);
	PUBLIC_FUNCTION(bool, IsUnderPoint, const Vec2F&);
	PROTECTED_FUNCTION(void, OnAssetsSelected);
	PROTECTED_FUNCTION(void, UpdateLayout, bool, bool);
	PROTECTED_FUNCTION(void, UpdateCuttingAssets);
	PROTECTED_FUNCTION(void, OnFocused);
	PROTECTED_FUNCTION(void, OnUnfocused);
	PROTECTED_FUNCTION(void, OnCursorPressed, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorReleased, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorPressBreak, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorStillDown, const Input::Cursor&);
	PROTECTED_FUNCTION(void, BeginSelecting);
	PROTECTED_FUNCTION(void, UpdateSelection, const Input::Cursor&);
	PROTECTED_FUNCTION(void, CompleteSelecting);
	PROTECTED_FUNCTION(void, RegActorsCreationAction);
	PROTECTED_FUNCTION(void, OnCursorMoved, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorRightMouseReleased, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnKeyReleased, const Input::Key&);
	PROTECTED_FUNCTION(void, InitializeContext);
	PROTECTED_FUNCTION(void, PrepareIconsPools);
	PROTECTED_FUNCTION(UIAssetIcon*, GetAssetIconFromPool, const String&);
	PROTECTED_FUNCTION(void, FreeAssetIconToPool, UIAssetIcon*);
	PROTECTED_FUNCTION(UIAssetIcon*, GetImageAssetIcon, const AssetInfo&);
	PROTECTED_FUNCTION(void, UpdateAssetsGridSize);
	PROTECTED_FUNCTION(void, OnIconDblClicked, UIAssetIcon*);
	PROTECTED_FUNCTION(void, OnContextCopyPressed);
	PROTECTED_FUNCTION(void, OnContextCutPressed);
	PROTECTED_FUNCTION(void, OnContextPastePressed);
	PROTECTED_FUNCTION(void, OnContextDeletePressed);
	PROTECTED_FUNCTION(void, OnContextOpenPressed);
	PROTECTED_FUNCTION(void, OnContextShowInExplorerPressed);
	PROTECTED_FUNCTION(void, OnContextImportPressed);
	PROTECTED_FUNCTION(void, OnContextCreateFolderPressed);
	PROTECTED_FUNCTION(void, OnContextCreatePrefabPressed);
	PROTECTED_FUNCTION(void, OnContextCreateScriptPressed);
	PROTECTED_FUNCTION(void, OnContextCreateAnimationPressed);
	PROTECTED_FUNCTION(void, InstantiateDraggingAssets);
	PROTECTED_FUNCTION(void, ClearInstantiatedDraggingAssets);
	PROTECTED_FUNCTION(Actor*, InstantiateAsset, const AssetInfo&);
	PROTECTED_FUNCTION(Actor*, InstantiateAsset, const ImageAssetRef&);
	PROTECTED_FUNCTION(Actor*, InstantiateAsset, const ActorAssetRef&);
	PROTECTED_FUNCTION(void, CheckPreloadedAssetsSaving);
	PROTECTED_FUNCTION(SelectDragObjectsVec, GetSelectedDragObjects);
	PROTECTED_FUNCTION(SelectDragObjectsVec, GetAllObjects);
	PROTECTED_FUNCTION(void, Select, SelectableDragableObject*);
	PROTECTED_FUNCTION(void, Select, SelectableDragableObject*, bool);
	PROTECTED_FUNCTION(void, Deselect, SelectableDragableObject*);
	PROTECTED_FUNCTION(void, AddSelectableObject, SelectableDragableObject*);
	PROTECTED_FUNCTION(void, RemoveSelectableObject, SelectableDragableObject*);
	PROTECTED_FUNCTION(void, OnSelectableObjectCursorReleased, SelectableDragableObject*, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnSelectableObjectBeganDragging, SelectableDragableObject*);
	PROTECTED_FUNCTION(void, BeginDragging, UIAssetIcon*);
	PROTECTED_FUNCTION(void, EndDragging, bool);
	PROTECTED_FUNCTION(void, UpdateDraggingGraphics);
	PROTECTED_FUNCTION(void, OnDragEnter, ISelectableDragableObjectsGroup*);
	PROTECTED_FUNCTION(void, OnDraggedAbove, ISelectableDragableObjectsGroup*);
	PROTECTED_FUNCTION(void, OnDragExit, ISelectableDragableObjectsGroup*);
	PROTECTED_FUNCTION(void, OnDropped, ISelectableDragableObjectsGroup*);
	PROTECTED_FUNCTION(void, OnDroppedFromThis);
	PROTECTED_FUNCTION(void, OnDroppedFromActorsTree, UIActorsTree*);
}
END_META;
