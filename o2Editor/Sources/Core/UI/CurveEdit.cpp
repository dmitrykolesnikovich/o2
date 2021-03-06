#include "CurveEdit.h"

#include "Application/Application.h"
#include "Render/Render.h"
#include "Render/Sprite.h"
#include "UI/ContextMenu.h"
#include "UI/EditBox.h"
#include "UI/HorizontalLayout.h"
#include "UI/HorizontalScrollBar.h"
#include "UI/Label.h"
#include "UI/UIManager.h"
#include "UI/VerticalLayout.h"
#include "UI/VerticalScrollBar.h"
#include "UI/Window.h"
#include "Utils/Clipboard.h"

namespace Editor
{
	UICurveEditor::UICurveEditor():
		UIFrameScrollView()
	{
		mReady = false;

		mSelectionSprite = mnew Sprite();
		mTextFont = FontRef("stdFont.ttf");
		mTextFont->CheckCharacters("0123456789.,+-", 10);

		InitializeTextDrawables();
		InitializeContextMenu();
		InitializeEditValueWindow();

		mTransformFrame.SetPivotEnabled(false);
		mTransformFrame.SetRotationEnabled(false);
		mTransformFrame.onTransformed = THIS_FUNC(OnTransformFrameTransformed);
		mTransformFrame.onPressed = THIS_FUNC(OnTransformBegin);
		mTransformFrame.onChangeCompleted = THIS_FUNC(OnTransformCompleted);
		mTransformFrame.messageFallDownListener = this;

		mBackColor = Color4(130, 130, 130, 255);

		mReady = true;
	}

	UICurveEditor::UICurveEditor(const UICurveEditor& other):
		UIFrameScrollView(other), mSelectionSprite(other.mSelectionSprite->Clone()), mTextFont(other.mTextFont)
	{
		mReady = false;

		mContextMenu = FindChild<UIContextMenu>();
		if (mContextMenu)
			delete mContextMenu;

		InitializeTextDrawables();
		InitializeContextMenu();
		InitializeEditValueWindow();
		RetargetStatesAnimations();

		mReady = true;
	}

	UICurveEditor::~UICurveEditor()
	{
		for (auto curve : mCurves)
			delete curve;

		delete mEditValueWindow;
		delete mSelectionSprite;
		delete mTextLeft;
		delete mTextRight;
		delete mTextTop;
		delete mTextBottom;
	}

	Editor::UICurveEditor& UICurveEditor::operator=(const UICurveEditor& other)
	{
		UIFrameScrollView::operator=(other);

		mReady = false;

		delete mSelectionSprite;
		delete mTextLeft;
		delete mTextRight;
		delete mTextTop;
		delete mTextBottom;

		mSelectionSprite = other.mSelectionSprite->Clone();
		mTextFont        = other.mTextFont;

		mMainHandleSample = other.mMainHandleSample;
		mSupportHandleSample = other.mSupportHandleSample;

		InitializeTextDrawables();
		InitializeEditValueWindow();
		RetargetStatesAnimations();

		mReady = true;

		return *this;
	}

	void UICurveEditor::Draw()
	{
		UIFrameScrollView::Draw();

		DrawTransformFrame();
		DrawHandles();
		DrawSelection();
	}

	void UICurveEditor::Update(float dt)
	{
		UIFrameScrollView::Update(dt);
	}

	void UICurveEditor::AddEditingCurve(const String& id, Curve* curve, const Color4& color /*= Color4::Green()*/)
	{
		CurveInfo* info = mnew CurveInfo();
		info->curveId = id;
		info->curve = curve;
		info->viewScale = Vec2F();
		info->UpdateApproximatedPoints();

		if (color == Color4::Green())
		{
			if (mCurves.IsEmpty())
				info->color = Color4::Green();
			else
				info->color = Color4::SomeColor(mCurves.Count());
		}
		else info->color = color;

		for (int i = 0; i < curve->GetKeys().Count(); i++)
			AddCurveKeyHandles(info, i);

		mCurves.Add(info);
		RecalculateViewArea();

		if (mCurves.Count() == 1)
			mViewCameraTargetScale = mAvailableArea.Size()/mViewCamera.GetSize();
	}

	void UICurveEditor::RemoveEditingCurve(Curve* curve)
	{
		for (auto info : mCurves)
		{
			if (info->curve == curve)
			{
				for (auto handle : info->handles)
				{
					mHandles.Remove(&handle->mainHandle);
					mHandles.Remove(&handle->leftSupportHandle);
					mHandles.Remove(&handle->rightSupportHandle);
					mSelectedHandles.Remove(&handle->mainHandle);
					mSelectedHandles.Remove(&handle->leftSupportHandle);
					mSelectedHandles.Remove(&handle->rightSupportHandle);
				}

				delete info;
				mCurves.Remove(info);
				break;
			}
		}
	}

	void UICurveEditor::RemoveEditingCurve(const String& id)
	{
		Curve* curve = FindCurve(id);
		if (curve)
			RemoveEditingCurve(curve);
	}

	void UICurveEditor::RemoveAllEditingCurves()
	{
		auto curves = mCurves;
		for (auto info : curves)
			RemoveEditingCurve(info->curve);
	}

	void UICurveEditor::AddCurvesRange(Curve* curveA, Curve* curveB, const Color4& color /*= Color4(-1, -1, -1, -1)*/)
	{

	}

	void UICurveEditor::AddCurvesRange(const String& idA, const String& idB, const Color4& color /*= Color4::Green()*/)
	{

	}

	void UICurveEditor::RemoveCurvesRange(Curve* curveA, Curve* curveB)
	{

	}

	void UICurveEditor::RemoveCurvesRange(const String& idA, const String& idB)
	{

	}

	void UICurveEditor::UpdateLayout(bool forcible /*= false*/, bool withChildren /*= true*/)
	{
		UIFrameScrollView::UpdateLayout(forcible, withChildren);

		UpdateLocalScreenTransforms();
		OnCameraTransformChanged();
	}

	void UICurveEditor::SetSelectionSpriteImage(const ImageAssetRef& image)
	{
		mSelectionSprite->LoadFromImage(image);
	}

	void UICurveEditor::SetTextFont(const FontRef& font)
	{
		mTextFont = font;
		mTextLeft->SetFont(mTextFont);
		mTextRight->SetFont(mTextFont);
		mTextTop->SetFont(mTextFont);
		mTextBottom->SetFont(mTextFont);
	}

	void UICurveEditor::SetMainHandleImages(const ImageAssetRef& regular, const ImageAssetRef& hover,
											const ImageAssetRef& pressed, const ImageAssetRef& selected)
	{
		mMainHandleSample = SelectableDragHandle(mnew Sprite(regular), mnew Sprite(hover),
												 mnew Sprite(pressed), mnew Sprite(selected));
	}

	void UICurveEditor::SetSupportHandleImages(const ImageAssetRef& regular, const ImageAssetRef& hover, const ImageAssetRef& pressed, const ImageAssetRef& selected)
	{
		mSupportHandleSample = SelectableDragHandle(mnew Sprite(regular), mnew Sprite(hover),
													mnew Sprite(pressed), mnew Sprite(selected));
	}

	void UICurveEditor::OnVisibleChanged()
	{
		if (mResVisible)
			mContextMenu->SetItemsMaxPriority();
		else
			mContextMenu->SetItemsMinPriority();
	}

	void UICurveEditor::OnScrolled(float scroll)
	{
		if (o2Input.IsKeyDown(VK_CONTROL))
			mViewCameraTargetScale.x *= 1.0f - (scroll*mViewCameraScaleSence);
		else if (o2Input.IsKeyDown(VK_SHIFT))
			mViewCameraTargetScale.y *= 1.0f - (scroll*mViewCameraScaleSence);
		else
			mViewCameraTargetScale *= 1.0f - (scroll*mViewCameraScaleSence);
	}

	Curve* UICurveEditor::FindCurve(const String& id)
	{
		for (auto curve : mCurves)
		{
			if (curve->curveId == id)
				return curve->curve;
		}

		return nullptr;
	}

	void UICurveEditor::InitializeContextMenu()
	{
		mContextMenu = o2UI.CreateWidget<UIContextMenu>();

		mContextMenu->AddItems({
			UIContextMenu::Item("Edit", THIS_FUNC(OnEditPressed), ImageAssetRef()),

			UIContextMenu::Item::Separator(),

			UIContextMenu::Item("Auto smooth", false, THIS_FUNC(OnAutoSmoothChecked)),
			UIContextMenu::Item("Flat", false, THIS_FUNC(OnFlatChecked)),
			UIContextMenu::Item("Free", false, THIS_FUNC(OnFreeChecked)),
			UIContextMenu::Item("Broken", false, THIS_FUNC(OnBrokenChecked)),
			UIContextMenu::Item("Discrete", false, THIS_FUNC(OnDiscreteChecked)),

			UIContextMenu::Item::Separator(),

			UIContextMenu::Item("Copy keys", THIS_FUNC(OnCopyPressed), ImageAssetRef(), ShortcutKeys('C', true)),
			UIContextMenu::Item("Cut keys", THIS_FUNC(OnCutPressed), ImageAssetRef(), ShortcutKeys('X', true)),
			UIContextMenu::Item("Paste keys", THIS_FUNC(OnPastePressed), ImageAssetRef(), ShortcutKeys('V', true)),
			UIContextMenu::Item("Delete keys", THIS_FUNC(OnDeletePressed), ImageAssetRef(), ShortcutKeys(VK_DELETE)),
			UIContextMenu::Item("Insert key", THIS_FUNC(OnInsertPressed), ImageAssetRef()),

			UIContextMenu::Item::Separator(),

			UIContextMenu::Item("Undo", THIS_FUNC(OnUndoPressed), ImageAssetRef(), ShortcutKeys('Z', true)),
			UIContextMenu::Item("Redo", THIS_FUNC(OnRedoPressed), ImageAssetRef(), ShortcutKeys('Z', true, true))
		});

		onShow = [&]() { mContextMenu->SetItemsMaxPriority(); };
		onHide = [&]() { mContextMenu->SetItemsMinPriority(); };

		AddChild(mContextMenu);
	}

	void UICurveEditor::InitializeTextDrawables()
	{
		mTextLeft = mnew Text(mTextFont);
		mTextLeft->SetHeight(10);
		mTextLeft->SetHorAlign(HorAlign::Left);
		mTextLeft->SetVerAlign(VerAlign::Bottom);

		mTextRight = mnew Text(mTextFont);
		mTextRight->SetHeight(10);
		mTextRight->SetHorAlign(HorAlign::Right);
		mTextRight->SetVerAlign(VerAlign::Bottom);

		mTextTop = mnew Text(mTextFont);
		mTextTop->SetHeight(10);
		mTextTop->SetHorAlign(HorAlign::Left);
		mTextTop->SetVerAlign(VerAlign::Bottom);
		mTextTop->SetAngleDegrees(-90.0f);

		mTextBottom = mnew Text(mTextFont);
		mTextBottom->SetHeight(10);
		mTextBottom->SetHorAlign(HorAlign::Right);
		mTextBottom->SetVerAlign(VerAlign::Bottom);
		mTextBottom->SetAngleDegrees(-90.0f);
	}

	void UICurveEditor::InitializeEditValueWindow()
	{
		mEditValueWindow = o2UI.AddWindow("Edit key");

		auto horLayout = o2UI.CreateHorLayout();
		horLayout->spacing = 10;

		auto positionVerLayout = o2UI.CreateVerLayout();
		positionVerLayout->AddChild(o2UI.CreateLabel("Position"));

		mEditValueWindowPosition = o2UI.CreateEditBox("singleline");
		mEditValueWindowPosition->onChangeCompleted = THIS_FUNC(OnEditKeyPositionChanged);
		positionVerLayout->AddChild(mEditValueWindowPosition);
		horLayout->AddChild(positionVerLayout);

		auto valueVerLayout = o2UI.CreateVerLayout();
		valueVerLayout->AddChild(o2UI.CreateLabel("Value"));

		mEditValueWindowValue = o2UI.CreateEditBox("singleline");
		mEditValueWindowValue->onChangeCompleted = THIS_FUNC(OnEditKeyValueChanged);
		valueVerLayout->AddChild(mEditValueWindowValue);
		horLayout->AddChild(valueVerLayout);

		mEditValueWindow->AddChild(horLayout);
		mEditValueWindow->layout.size = Vec2F(200, 80);
		mEditValueWindow->Hide(true);
	}

	void UICurveEditor::RecalculateViewArea()
	{
		if (mCurves.IsEmpty())
		{
			mAvailableArea = RectF(0, 0, 1, 1);
			return;
		}

		Vec2F firstPoint = mCurves.Last()->approximatedPoints.Last();
		mAvailableArea = RectF(firstPoint, firstPoint);

		for (auto curve : mCurves)
		{
			for (auto& p : curve->approximatedPoints)
			{
				mAvailableArea.left = Math::Min(mAvailableArea.left, p.x);
				mAvailableArea.right = Math::Max(mAvailableArea.right, p.x);
				mAvailableArea.top = Math::Max(mAvailableArea.top, p.y);
				mAvailableArea.bottom = Math::Min(mAvailableArea.bottom, p.y);
			}
		}

		float bordersCoef = 1.5f;
		Vec2F size = mAvailableArea.Size();
		mAvailableArea.left -= size.x*bordersCoef;
		mAvailableArea.right += size.x*bordersCoef;
		mAvailableArea.top += size.y*bordersCoef;
		mAvailableArea.bottom -= size.y*bordersCoef;

		mHorScrollbar->SetValueRange(mAvailableArea.left, mAvailableArea.right);
		mVerScrollbar->SetValueRange(mAvailableArea.bottom, mAvailableArea.top);
	}

	void UICurveEditor::RedrawContent()
	{
		DrawGrid();
		DrawCurves();

// 		o2Render.DrawRectFrame(mAvailableArea);
// 
// 		RectF outside = mAvailableArea;
// 		outside.left -= mAvailableArea.Width()*0.1f;
// 		outside.right += mAvailableArea.Width()*0.1f;
// 		outside.bottom -= mAvailableArea.Height()*0.1f;
// 		outside.top += mAvailableArea.Height()*0.1f;
// 
// 		o2Render.DrawRectFrame(outside, Color4::Green());
// 
// 		RectF inside = mAvailableArea;
// 		inside.left += mAvailableArea.Width()*0.1f;
// 		inside.right -= mAvailableArea.Width()*0.1f;
// 		inside.bottom += mAvailableArea.Height()*0.1f;
// 		inside.top -= mAvailableArea.Height()*0.1f;
// 
// 		o2Render.DrawRectFrame(inside, Color4::Red());
	}

	void UICurveEditor::DrawGrid()
	{
		float cameraMaxSize = Math::Max(mViewCamera.size->x*mViewCamera.scale->x,
										mViewCamera.size->y*mViewCamera.scale->y);

		float x = cameraMaxSize / 2.0f;
		float minCellSize = 0.000001f;
		float maxCellSize = 1000000.0f;
		float cellSize = minCellSize;
		while (cellSize < maxCellSize)
		{
			float next = cellSize*10.0f;
			if (x > cellSize && x <= next)
				break;

			cellSize = next;
		}

		Vec2F gridOrigin(Math::Round(mViewCamera.position->x / cellSize)*cellSize,
						 Math::Round(mViewCamera.position->y / cellSize)*cellSize);

		int cellsCount = Math::CeilToInt(cameraMaxSize / cellSize);
		float tenCeilsSize = cellSize*10.0f;
		float screenCellSize = cellSize / mViewCamera.scale->x;
		Color4 cellColorSmoothed = Math::Lerp(mGridColor, mBackColor, 0.7f);

		mTextLeft->SetScale(mViewCamera.GetScale());
		mTextRight->SetScale(mViewCamera.GetScale());

		Vec2F invScale(mViewCamera.GetScale().y, mViewCamera.GetScale().x);
		mTextTop->SetScale(invScale);
		mTextBottom->SetScale(invScale);

		for (int i = -cellsCount / 2; i < cellsCount / 2; i++)
		{
			float d = (float)i*cellSize;
			Vec2F dorigin = gridOrigin + Vec2F(d, d);

			float rdx = Math::Abs(dorigin.x / tenCeilsSize - Math::Floor(dorigin.x / tenCeilsSize));
			float rdy = Math::Abs(dorigin.y / tenCeilsSize - Math::Floor(dorigin.y / tenCeilsSize));
			bool xTen = rdx < 0.05f || rdx > 0.95f;
			bool yTen = rdy < 0.05f || rdy > 0.95f;

			Vec2F xBegin = Vec2F(-cameraMaxSize, d) + gridOrigin;
			Vec2F xEnd = Vec2F(cameraMaxSize, d) + gridOrigin;
			Vec2F yBegin = Vec2F(d, -cameraMaxSize) + gridOrigin;
			Vec2F yEnd = Vec2F(d, cameraMaxSize) + gridOrigin;

			o2Render.DrawLine(xBegin, xEnd, yTen ? mGridColor : cellColorSmoothed);
			o2Render.DrawLine(yBegin, yEnd, xTen ? mGridColor : cellColorSmoothed);
		}

		char buf[255];
		for (int i = -cellsCount / 2; i < cellsCount / 2; i++)
		{
			float x = (float)i*cellSize + gridOrigin.x;
			float y = (float)i*cellSize + gridOrigin.y;

			sprintf(buf, "%.1f", (Math::Round(x*10.0f)/10.0f));
			String xCaption = buf;

			sprintf(buf, "%.1f", (Math::Round(y*10.0f)/10.0f));
			String yCaption = buf;

			mTextLeft->SetText(yCaption);
			mTextLeft->SetPosition(Vec2F(mViewCamera.GetRect().left, y));
			mTextLeft->Draw();

			mTextRight->SetText(yCaption);
			mTextRight->SetPosition(Vec2F(mViewCamera.GetRect().right, y));
			mTextRight->Draw();

			mTextTop->SetText(xCaption);
			mTextTop->SetPosition(Vec2F(x, mViewCamera.GetRect().top));
			mTextTop->Draw();

			mTextBottom->SetText(xCaption);
			mTextBottom->SetPosition(Vec2F(x, mViewCamera.GetRect().bottom));
			mTextBottom->Draw();
		}
	}

	void UICurveEditor::DrawCurves()
	{
		for (auto curve : mCurves)
		{
			if (curve->approximatedPoints.IsEmpty())
				continue;

			//o2Render.DrawLine(curve->approximatedPoints, curve->color);

			float cameraLeftPos = mViewCamera.GetRect().left;
			float cameraRightPos = mViewCamera.GetRect().right;
			for (int i = 1; i < curve->approximatedPoints.Count(); i++)
			{
				Vec2F& a = curve->approximatedPoints[i - 1];
				Vec2F& b = curve->approximatedPoints[i];

				if (b.x < cameraLeftPos)
					continue;

				if (a.x > cameraRightPos)
					break;

				o2Render.DrawLine(a, b, curve->color);
			}

// 			int idx = 0;
// 			for (auto key : curve->curve->GetKeys())
// 			{
// 				mTextLeft->position = Vec2F(key.position, key.value - 0.1f);
// 				mTextLeft->text = (String)idx + ": \n" + (String)key.leftSupportPosition + ", " + (String)key.leftSupportValue +
// 					"\n" + (String)key.rightSupportPosition + ", " + (String)key.rightSupportValue;
// 				mTextLeft->Draw();
// 				idx++;
// 			}
		}
	}

	void UICurveEditor::DrawHandles()
	{
		RectF camRect = mViewCamera.GetRect();
		for (auto curve : mCurves)
		{
			for (auto keyHandles : curve->handles)
				keyHandles->Draw(camRect);
		}
	}

	void UICurveEditor::DrawSelection()
	{
		if (mIsPressed)
		{
			mSelectionSprite->rect = RectF(LocalToScreenPoint(mSelectingPressedPoint), o2Input.cursorPos);
			mSelectionSprite->Draw();
		}
	}

	void UICurveEditor::DrawTransformFrame()
	{
		if (!mTransformFrameVisible)
			return;

		Vec2F borders(10, 10);

		mTransformFrame.SetBasis(Basis(LocalToScreenPoint(mTransformFrameBasis.offs) - borders,
								 mTransformFrameBasis.xv/mViewCamera.GetScale() + Vec2F(borders.x*2.0f, 0),
								 mTransformFrameBasis.yv/mViewCamera.GetScale() + Vec2F(0, borders.y*2.0f)));

		mTransformFrame.Draw();

		if (o2Input.IsKeyDown(VK_CONTROL))
		{
			Vec2F left = mTransformFrame.GetCurrentBasis().offs;
			Vec2F right = mTransformFrame.GetCurrentBasis().offs + mTransformFrame.GetCurrentBasis().xv;
			RectF rect = layout.GetAbsoluteRect();

			o2Render.DrawLine(Vec2F(right.x, rect.bottom), Vec2F(right.x, rect.top), mTransformFrame.GetFrameColor());
			o2Render.DrawLine(Vec2F(left.x, rect.bottom), Vec2F(left.x, rect.top), mTransformFrame.GetFrameColor());
		}
	}

	void UICurveEditor::AddCurveKeyHandles(CurveInfo* info, int keyId)
	{
		KeyHandles* keyHandles = mnew KeyHandles(mMainHandleSample, mSupportHandleSample, this);
		keyHandles->curveKeyIdx = keyId;

		Curve::Key curveKey = info->curve->GetKeyAt(keyId);
		Curve::Key lastCurveKey = info->curve->GetKeyAt(Math::Max(0, keyId - 1));
		Curve::Key nextCurveKey = info->curve->GetKeyAt(Math::Min(keyId + 1, info->curve->GetKeys().Count()));

		// main handle
		keyHandles->mainHandle.SetPosition(Vec2F(curveKey.position, curveKey.value));
		keyHandles->mainHandle.onChangedPos = [=](const Vec2F& pos) { OnCurveKeyMainHandleDragged(info, keyHandles, pos); };
		keyHandles->mainHandle.localToScreenTransformFunc = [&](const Vec2F& p) { return LocalToScreenPoint(p); };
		keyHandles->mainHandle.screenToLocalTransformFunc = [&](const Vec2F& p) { return ScreenToLocalPoint(p); };
		keyHandles->mainHandle.onRightButtonReleased = THIS_FUNC(OnCursorRightMouseReleased);
		keyHandles->mainHandle.onPressed = THIS_FUNC(OnTransformBegin);
		keyHandles->mainHandle.onChangeCompleted = THIS_FUNC(OnTransformCompleted);


		// left support handle
		keyHandles->leftSupportHandle.SetPosition(Vec2F(curveKey.position + curveKey.leftSupportPosition,
												  curveKey.value + curveKey.leftSupportValue));

		keyHandles->leftSupportHandle.localToScreenTransformFunc =
			[&](const Vec2F& p) { return LocalToScreenPoint(p); };

		keyHandles->leftSupportHandle.screenToLocalTransformFunc =
			[&](const Vec2F& p) { return ScreenToLocalPoint(p); };

		keyHandles->leftSupportHandle.onChangedPos =
			[=](const Vec2F& pos) { OnCurveKeyLeftSupportHandleDragged(info, keyHandles, pos); };

		keyHandles->leftSupportHandle.checkPositionFunc =
			[=](const Vec2F& pos) { return CheckLeftSupportHandlePosition(info, keyHandles, pos); };

		keyHandles->leftSupportHandle.enabled = false;
		keyHandles->leftSupportHandle.onRightButtonReleased = THIS_FUNC(OnCursorRightMouseReleased);
		keyHandles->leftSupportHandle.onPressed = THIS_FUNC(OnTransformBegin);
		keyHandles->leftSupportHandle.onChangeCompleted = THIS_FUNC(OnTransformCompleted);


		// right support handle
		keyHandles->rightSupportHandle.SetPosition(Vec2F(curveKey.position + curveKey.rightSupportPosition,
												   curveKey.value + curveKey.rightSupportValue));

		keyHandles->rightSupportHandle.localToScreenTransformFunc =
			[&](const Vec2F& p) { return LocalToScreenPoint(p); };

		keyHandles->rightSupportHandle.screenToLocalTransformFunc =
			[&](const Vec2F& p) { return ScreenToLocalPoint(p); };

		keyHandles->rightSupportHandle.onChangedPos =
			[=](const Vec2F& pos) { OnCurveKeyRightSupportHandleDragged(info, keyHandles, pos); };

		keyHandles->rightSupportHandle.checkPositionFunc =
			[=](const Vec2F& pos) { return CheckRightSupportHandlePosition(info, keyHandles, pos); };

		keyHandles->rightSupportHandle.enabled = false;
		keyHandles->rightSupportHandle.onRightButtonReleased = THIS_FUNC(OnCursorRightMouseReleased);
		keyHandles->rightSupportHandle.onPressed = THIS_FUNC(OnTransformBegin);
		keyHandles->rightSupportHandle.onChangeCompleted = THIS_FUNC(OnTransformCompleted);

		for (int i = keyId; i < info->handles.Count(); i++)
			info->handles[i]->curveKeyIdx++;

		info->handles.Insert(keyHandles, keyId);

		mHandles.Add(&keyHandles->mainHandle);
		mHandles.Add(&keyHandles->leftSupportHandle);
		mHandles.Add(&keyHandles->rightSupportHandle);

		mSupportHandles.Add(&keyHandles->leftSupportHandle);
		mSupportHandles.Add(&keyHandles->rightSupportHandle);

		keyHandles->mainHandle.SetSelectionGroup(this);
		keyHandles->leftSupportHandle.SetSelectionGroup(this);
		keyHandles->rightSupportHandle.SetSelectionGroup(this);
	}

	void UICurveEditor::RemoveCurveKeyHandles(CurveInfo* info, int keyId)
	{
		for (int i = keyId + 1; i < info->handles.Count(); i++)
			info->handles[i]->curveKeyIdx--;

		mHandles.Remove(&info->handles[keyId]->mainHandle);
		mHandles.Remove(&info->handles[keyId]->leftSupportHandle);
		mHandles.Remove(&info->handles[keyId]->rightSupportHandle);

		mSupportHandles.Remove(&info->handles[keyId]->leftSupportHandle);
		mSupportHandles.Remove(&info->handles[keyId]->rightSupportHandle);

		delete info->handles[keyId];
		info->handles.RemoveAt(keyId);
	}

	void UICurveEditor::OnCurveKeyMainHandleDragged(CurveInfo* info, KeyHandles* handles, const Vec2F& position)
	{
		Curve::Key key = info->curve->GetKeyAt(handles->curveKeyIdx);

		Vec2F initialDragPoint = handles->mainHandle.GetDraggingBeginPosition();

		if (handles->mainHandle.IsPressed())
		{
			if (o2Input.IsKeyDown(VK_CONTROL) && o2Input.IsKeyDown(VK_SHIFT))
			{
				key.position = Math::Round(position.x*10.0f)/10.0f;
				key.value = Math::Round(position.y*10.0f)/10.0f;
			}
			else if (o2Input.IsKeyDown(VK_CONTROL))
			{
				key.position = position.x;
				key.value = initialDragPoint.y;
			}
			else if (o2Input.IsKeyDown(VK_SHIFT))
			{
				key.position = initialDragPoint.x;
				key.value = position.y;
			}
			else
			{
				key.position = position.x;
				key.value = position.y;
			}
		}
		else
		{
			key.position = position.x;
			key.value = position.y;
		}

		int newKeyIdx = 0;
		for (auto& k : info->curve->GetKeys())
		{
			if (k.position > key.position)
				break;

			newKeyIdx++;
		}

		if (newKeyIdx > handles->curveKeyIdx)
			newKeyIdx--;

		if (newKeyIdx != handles->curveKeyIdx)
		{
			info->curve->RemoveKeyAt(handles->curveKeyIdx);
			info->curve->InsertKey(key);

			info->handles.RemoveAt(handles->curveKeyIdx);
			info->handles.Insert(handles, newKeyIdx);

			for (int i = 0; i < info->handles.Count(); i++)
				info->handles[i]->curveKeyIdx = i;
		}

		info->curve->SetKey(key, handles->curveKeyIdx);

		info->UpdateHandles();
		info->UpdateApproximatedPoints();

		CheckHandlesVisible();
		UpdateTransformFrame();
		RecalculateViewArea();
		mNeedRedraw = true;
	}

	void UICurveEditor::OnCurveKeyLeftSupportHandleDragged(CurveInfo* info, KeyHandles* handles, const Vec2F& position)
	{
		Curve::Key key = info->curve->GetKeyAt(handles->curveKeyIdx);
		Curve::Key prevKey = info->curve->GetKeyAt(Math::Max(handles->curveKeyIdx - 1, 0));

		if (o2Input.IsKeyDown(VK_CONTROL) && o2Input.IsKeyDown(VK_SHIFT))
		{
			Vec2F initialDragPoint = handles->leftSupportHandle.GetDraggingBeginPosition();
			float dst = (initialDragPoint - Vec2F(key.position, key.value)).Length();
			Vec2F v = (position - Vec2F(key.position, key.value)).Normalized()*dst;
			key.leftSupportPosition = v.x;
			key.leftSupportValue = v.y;
		}
		else if (o2Input.IsKeyDown(VK_CONTROL))
		{
			key.leftSupportPosition = position.x - key.position;
			key.leftSupportValue = 0;
		}
		else if (o2Input.IsKeyDown(VK_SHIFT))
		{
			Vec2F initialDragPoint = handles->leftSupportHandle.GetDraggingBeginPosition();

			Vec2F v = initialDragPoint - Vec2F(key.position, key.value);
			float dst = (position - Vec2F(key.position, key.value)).Length();
			v = v.Normalized()*dst;
			key.leftSupportPosition = v.x;
			key.leftSupportValue = v.y;
		}
		else
		{
			key.leftSupportPosition = position.x - key.position;
			key.leftSupportValue = position.y - key.value;
		}

		if (key.supportsType == Curve::Key::Type::Flat || key.supportsType == Curve::Key::Type::Discrete ||
			key.supportsType == Curve::Key::Type::Smooth)
		{
			key.supportsType = Curve::Key::Type::Free;
		}

		if (key.supportsType == Curve::Key::Type::Free && handles->curveKeyIdx < info->curve->GetKeys().Count() - 1)
		{
			Curve::Key nextKey = info->curve->GetKeyAt(Math::Min(handles->curveKeyIdx + 1,
													   info->curve->GetKeys().Count() - 1));

			key.rightSupportPosition = -key.leftSupportPosition;
			key.rightSupportValue = -key.leftSupportValue;
		}

		info->curve->SetKey(key, handles->curveKeyIdx);

		info->UpdateHandles();
		info->UpdateApproximatedPoints();

		CheckHandlesVisible();
		RecalculateViewArea();
		mNeedRedraw = true;
	}

	void UICurveEditor::OnCurveKeyRightSupportHandleDragged(CurveInfo* info, KeyHandles* handles, const Vec2F& position)
	{
		Curve::Key key = info->curve->GetKeyAt(handles->curveKeyIdx);
		Curve::Key nextKey = info->curve->GetKeyAt(Math::Min(handles->curveKeyIdx + 1, info->curve->GetKeys().Count() - 1));

		if (o2Input.IsKeyDown(VK_CONTROL) && o2Input.IsKeyDown(VK_SHIFT))
		{
			Vec2F initialDragPoint = handles->rightSupportHandle.GetDraggingBeginPosition();
			float dst = (initialDragPoint - Vec2F(key.position, key.value)).Length();
			Vec2F v = (position - Vec2F(key.position, key.value)).Normalized()*dst;
			key.rightSupportPosition = v.x;
			key.rightSupportValue = v.y;
		}
		else if (o2Input.IsKeyDown(VK_CONTROL))
		{
			key.rightSupportPosition = position.x - key.position;
			key.rightSupportValue = 0;
		}
		else if (o2Input.IsKeyDown(VK_SHIFT))
		{
			Vec2F initialDragPoint = handles->rightSupportHandle.GetDraggingBeginPosition();

			Vec2F v = initialDragPoint - Vec2F(key.position, key.value);
			float dst = (position - Vec2F(key.position, key.value)).Length();
			v = v.Normalized()*dst;
			key.rightSupportPosition = v.x;
			key.rightSupportValue = v.y;
		}
		else
		{
			key.rightSupportPosition = position.x - key.position;
			key.rightSupportValue = position.y - key.value;
		}

		if (key.supportsType == Curve::Key::Type::Flat || key.supportsType == Curve::Key::Type::Discrete ||
			key.supportsType == Curve::Key::Type::Smooth)
		{
			key.supportsType = Curve::Key::Type::Free;
		}

		if (key.supportsType == Curve::Key::Type::Free && handles->curveKeyIdx > 0)
		{
			Curve::Key prevKey = info->curve->GetKeyAt(Math::Max(handles->curveKeyIdx - 1, 0));

			Vec2F rightSupportVector = Vec2F(key.rightSupportPosition, key.rightSupportValue);
			Vec2F leftSupportVector = rightSupportVector.Inverted();

			key.leftSupportPosition = -key.rightSupportPosition;
			key.leftSupportValue = -key.rightSupportValue;

			handles->leftSupportHandle.position = Vec2F(key.position + key.leftSupportPosition,
														key.value + key.leftSupportValue);
		}

		info->curve->SetKey(key, handles->curveKeyIdx);

		info->UpdateHandles();
		info->UpdateApproximatedPoints();

		CheckHandlesVisible();
		RecalculateViewArea();
		mNeedRedraw = true;
	}

	Vec2F UICurveEditor::CheckLeftSupportHandlePosition(CurveInfo* info, KeyHandles* handles, const Vec2F& position)
	{
		Curve::Key key = info->curve->GetKeyAt(handles->curveKeyIdx);
		Curve::Key prevKey = info->curve->GetKeyAt(Math::Max(handles->curveKeyIdx - 1, 0));

		Vec2F relativePos = position - Vec2F(key.position, key.value);

		if (relativePos.x > 0.0f)
			relativePos.x = 0.0f;

		if (relativePos.x < prevKey.position - key.position)
			relativePos *= (prevKey.position - key.position)/relativePos.x;

		return Vec2F(key.position, key.value) + relativePos;
	}

	Vec2F UICurveEditor::CheckRightSupportHandlePosition(CurveInfo* info, KeyHandles* handles, const Vec2F& position)
	{
		Curve::Key key = info->curve->GetKeyAt(handles->curveKeyIdx);
		Curve::Key nextKey = info->curve->GetKeyAt(Math::Min(handles->curveKeyIdx + 1, info->curve->GetKeys().Count() - 1));

		Vec2F relativePos = position - Vec2F(key.position, key.value);

		if (relativePos.x < 0.0f)
			relativePos.x = 0.0f;

		if (relativePos.x > nextKey.position - key.position)
			relativePos *= (nextKey.position - key.position)/relativePos.x;

		return Vec2F(key.position, key.value) + relativePos;
	}

	void UICurveEditor::OnCursorDblClicked(const Input::Cursor& cursor)
	{
		const float createPointDistanceThreshold = 7;
		const float supportValuesCoef = 0.15f;

		CurveInfo* clickedCurveInfo = nullptr;
		Curve::Key newKey;
		Vec2F localCursorPos = ScreenToLocalPoint(cursor.position);

		for (auto info : mCurves)
		{
			auto& keys = info->curve->GetKeys();
			int keyIdx = 0;
			for (auto& key : keys)
			{
				if (keyIdx == 0)
				{
					keyIdx++;
					continue;
				}

				const Vec2F* points = key.GetApproximatedPoints();
				for (int i = 1; i < key.GetApproximatedPointsCount(); i++)
				{
					Vec2F a = points[i - 1], b = points[i];
					Vec2F ab = b - a;
					float abl = ab.Length();
					Vec2F abn = ab/abl;
					Vec2F abnp = abn.Perpendicular();
					Vec2F ac = cursor.position - LocalToScreenPoint(a);
					Vec2F lac = ScreenToLocalPoint(cursor.position) - a;

					float pointDistance = (LocalToScreenPoint(b) - cursor.position).Length();
					float lineDistance = Math::Abs(abnp.Dot(ac));
					float proj = abn.Dot(lac);

					if (pointDistance < createPointDistanceThreshold ||
						(lineDistance < createPointDistanceThreshold && proj > 0.0f && proj < abl))
					{
						const Curve::Key& lastKey = keys[keyIdx - 1];

						Vec2F p = abnp*(abnp.Dot(localCursorPos - a)) + localCursorPos;
						newKey.position = p.x;
						newKey.value = p.y;
						newKey.supportsType = Curve::Key::Type::Smooth;

						clickedCurveInfo = info;

						break;
					}
				}

				if (clickedCurveInfo)
					break;

				keyIdx++;
			}

			if (clickedCurveInfo)
				break;
		}

		bool addedKey = false;
		if (clickedCurveInfo)
		{
			int idx = clickedCurveInfo->curve->InsertKey(newKey);

			AddCurveKeyHandles(clickedCurveInfo, idx);
			clickedCurveInfo->UpdateApproximatedPoints();

			mNeedRedraw = true;

			SelectHandle(&clickedCurveInfo->handles[idx]->mainHandle);
			CheckHandlesVisible();

			addedKey = true;
		}
		else if (mCurves.Count() == 1)
		{
			clickedCurveInfo = mCurves[0];
			newKey.position = localCursorPos.x;
			newKey.value = localCursorPos.y;
			int idx = clickedCurveInfo->curve->InsertKey(newKey);

			AddCurveKeyHandles(mCurves[0], idx);
			clickedCurveInfo->UpdateApproximatedPoints();

			mNeedRedraw = true;

			SelectHandle(&clickedCurveInfo->handles[idx]->mainHandle);
			CheckHandlesVisible();

			addedKey = true;
		}

		if (addedKey)
		{
			CurveKeysInfosVec keyInfos;
			keyInfos.Add(CurveKeysInfo());
			keyInfos.Last().curveId = clickedCurveInfo->curveId;
			keyInfos.Last().keys.Add(newKey);

			AddKeysAction* action = mnew AddKeysAction(keyInfos, this);
			DoneAction(action);
		}
	}

	void UICurveEditor::SmoothKey(CurveInfo* info, int idx)
	{
		Curve::Key key = info->curve->GetKeyAt(idx);
		Vec2F thisKeyPoint(key.position, key.value);

		const float autoSmoothCoef = 0.5f;

		if (idx == 0)
		{
			Curve::Key nextKey = info->curve->GetKeyAt(Math::Min(idx + 1, info->curve->GetKeys().Count()));

			Vec2F nextKeyPoint(nextKey.position, nextKey.value);
			Vec2F supportVec = (nextKeyPoint - thisKeyPoint)*autoSmoothCoef;

			key.rightSupportPosition = supportVec.x;
			key.rightSupportValue = supportVec.y;
		}
		else if (idx == info->curve->GetKeys().Count() - 1)
		{
			Curve::Key lastKey = info->curve->GetKeyAt(Math::Max(0, idx - 1));

			Vec2F lastKeyPoint(lastKey.position, lastKey.value);
			Vec2F supportVec = (lastKeyPoint - thisKeyPoint)*autoSmoothCoef;

			key.leftSupportPosition = supportVec.x;
			key.leftSupportValue = supportVec.y;
		}
		else
		{
			Curve::Key lastKey = info->curve->GetKeyAt(Math::Max(0, idx - 1));
			Curve::Key nextKey = info->curve->GetKeyAt(Math::Min(idx + 1, info->curve->GetKeys().Count()));

			Vec2F lastKeyPoint(lastKey.position, lastKey.value);
			Vec2F thisKeyPoint(key.position, key.value);
			Vec2F nextKeyPoint(nextKey.position, nextKey.value);

			Vec2F thisToLast = lastKeyPoint - thisKeyPoint;
			Vec2F thisToNext = nextKeyPoint - thisKeyPoint;

			float supportLength = Math::Min(thisToNext.Length(), thisToLast.Length())*autoSmoothCoef;

			Vec2F supportVec = Math::CalculateEllipseTangent(lastKeyPoint, thisKeyPoint, nextKeyPoint)*supportLength;

			key.leftSupportPosition = -supportVec.x; key.leftSupportValue = -supportVec.y;
			key.rightSupportPosition = supportVec.x; key.rightSupportValue = supportVec.y;
		}

		info->curve->SetKey(key, idx);

		info->UpdateApproximatedPoints();

		info->handles[idx]->leftSupportHandle.SetPosition(Vec2F(key.position + key.leftSupportPosition,
														  key.value + key.leftSupportValue));

		info->handles[idx]->rightSupportHandle.SetPosition(Vec2F(key.position + key.rightSupportPosition,
														   key.value + key.rightSupportValue));

		if (idx > 0)
		{
			Curve::Key lastKey = info->curve->GetKeyAt(Math::Max(0, idx - 1));

			info->handles[idx - 1]->rightSupportHandle.SetPosition(
				Vec2F(lastKey.position + lastKey.rightSupportPosition, lastKey.value + lastKey.rightSupportValue));
		}

		if (idx < info->curve->GetKeys().Count() - 1)
		{
			Curve::Key nextKey = info->curve->GetKeyAt(Math::Min(idx + 1, info->curve->GetKeys().Count()));

			info->handles[idx + 1]->leftSupportHandle.SetPosition(
				Vec2F(nextKey.position + nextKey.leftSupportPosition, nextKey.value + nextKey.leftSupportValue));
		}

		RecalculateViewArea();

		mNeedRedraw = true;
	}

	void UICurveEditor::OnCursorPressed(const Input::Cursor& cursor)
	{
		Focus();

		mSelectingPressedPoint = ScreenToLocalPoint(cursor.position);

		if (!o2Input.IsKeyDown(VK_CONTROL))
		{
			auto handles = mSelectedHandles;
			for (auto handle : handles)
				handle->SetSelected(false);

			mSelectedHandles.Clear();
		}
	}

	void UICurveEditor::OnCursorReleased(const Input::Cursor& cursor)
	{
		mSelectedHandles.Add(mSelectingHandlesBuf);
		mSelectingHandlesBuf.Clear();
		UpdateTransformFrame();
		CheckHandlesVisible();
	}

	void UICurveEditor::OnCursorStillDown(const Input::Cursor& cursor)
	{
		for (auto handle : mSelectingHandlesBuf)
			SetHandleSelectedState(handle, false);

		mSelectingHandlesBuf.Clear();

		RectF selectionLocalRect(mSelectingPressedPoint, ScreenToLocalPoint(cursor.position));

		for (auto handle : mHandles)
		{
			if (handle->IsEnabled() && selectionLocalRect.IsInside(handle->GetPosition()) &&
				!mSelectedHandles.Contains(handle))
			{
				mSelectingHandlesBuf.Add(handle);
				SetHandleSelectedState(handle, true);
			}
		}
	}

	void UICurveEditor::OnCursorRightMouseStayDown(const Input::Cursor& cursor)
	{
		if (cursor.delta.Length() > 0.1f)
			mIsViewScrolling = true;

		UIScrollView::OnCursorRightMouseStayDown(cursor);
	}

	void UICurveEditor::OnCursorRightMouseReleased(const Input::Cursor& cursor)
	{
		Focus();

		if (!mIsViewScrolling)
		{
			Curve::Key::Type supportsType = Curve::Key::Type::Free;
			bool supportsDifferent = false;
			bool first = true;
			bool someSelected = false;

			for (auto curve : mCurves)
			{
				for (auto handles : curve->handles)
				{
					if (!handles->mainHandle.IsSelected())
						continue;

					someSelected = true;

					if (first)
					{
						first = false;
						supportsType = curve->curve->GetKeyAt(handles->curveKeyIdx).supportsType;
					}
					else if (supportsType != curve->curve->GetKeyAt(handles->curveKeyIdx).supportsType)
					{
						supportsDifferent = true;
						break;
					}
				}

				if (supportsDifferent)
					break;
			}

			if (supportsDifferent || !someSelected)
			{
				for (int i = 0; i < 5; i++)
					mContextMenu->SetItemChecked(i, false);
			}
			else
			{
				for (int i = 0; i < 5; i++)
					mContextMenu->SetItemChecked(i, i == (int)supportsType);
			}

			mContextMenu->Show();
		}

		mIsViewScrolling = false;

		UIScrollView::OnCursorRightMouseReleased(cursor);
	}

	void UICurveEditor::CheckHandlesVisible()
	{
		for (auto info : mCurves)
		{
			for (auto handles : info->handles)
			{
				handles->leftSupportHandle.enabled = (handles->mainHandle.IsSelected() ||
													  handles->leftSupportHandle.IsSelected() ||
													  handles->rightSupportHandle.IsSelected()) && handles->curveKeyIdx > 0;

				handles->rightSupportHandle.enabled = (handles->mainHandle.IsSelected() ||
													   handles->leftSupportHandle.IsSelected() ||
													   handles->rightSupportHandle.IsSelected()) && handles->curveKeyIdx < info->handles.Count() - 1;
			}
		}
	}

	void UICurveEditor::UpdateTransformFrame()
	{
		mTransformFrameVisible = IsTransformFrameVisible();

		if (!mTransformFrameVisible)
			return;

		RectF aabb(mSelectedHandles[0]->GetPosition(), mSelectedHandles[0]->GetPosition());

		for (auto handle : mSelectedHandles)
		{
			if (mSupportHandles.Contains(handle))
				continue;

			aabb.left   = Math::Min(handle->GetPosition().x, aabb.left);
			aabb.right  = Math::Max(handle->GetPosition().x, aabb.right);
			aabb.top    = Math::Max(handle->GetPosition().y, aabb.top);
			aabb.bottom = Math::Min(handle->GetPosition().y, aabb.bottom);
		}

		mTransformFrameBasis = Basis(aabb.LeftBottom(), Vec2F::Right()*aabb.Width(), Vec2F::Up()*aabb.Height());
	}

	bool UICurveEditor::IsTransformFrameVisible() const
	{
		int selectedMainHandles = 0;
		for (auto handle : mSelectedHandles)
		{
			if (!mSupportHandles.Contains(handle))
				selectedMainHandles++;
		}

		return selectedMainHandles > 1;
	}

	void UICurveEditor::OnHandleCursorReleased(SelectableDragHandle* handle, const Input::Cursor& cursor)
	{
		SelectableDragHandlesGroup::OnHandleCursorReleased(handle, cursor);
		UpdateTransformFrame();
		CheckHandlesVisible();
	}

	void UICurveEditor::OnHandleBeganDragging(SelectableDragHandle* handle)
	{
		if (mSupportHandles.Contains(handle))
			return;

		SelectableDragHandlesGroup::OnHandleBeganDragging(handle);
	}

	void UICurveEditor::OnHandleMoved(SelectableDragHandle* handle, const Input::Cursor& cursor)
	{
		if (mSupportHandles.Contains(handle))
		{
			if (!handle->IsSelected())
				return;

			for (auto handle : mSupportHandles)
			{
				if (!handle->IsSelected())
					continue;

				handle->SetDragPosition(handle->screenToLocalTransformFunc(cursor.position) + handle->GetDraggingOffset());
				handle->onChangedPos(handle->GetPosition());
			}

			return;
		}

		SelectableDragHandlesGroup::OnHandleMoved(handle, cursor);
	}

	void UICurveEditor::OnHandleCompletedChange(SelectableDragHandle* handle)
	{
		OnTransformCompleted();
	}

	void UICurveEditor::SetSelectedKeysSupportsType(Curve::Key::Type type)
	{
		for (auto info : mCurves)
		{
			auto keys = info->curve->GetKeys();

			for (auto handles : info->handles)
			{
				if (handles->mainHandle.IsSelected())
				{
					Curve::Key& key = keys[handles->curveKeyIdx];
					key.supportsType = type;

					switch (type)
					{
						case Curve::Key::Type::Flat:
						key.leftSupportValue = 0;
						key.rightSupportValue = 0;
						break;

						case Curve::Key::Type::Free:
						key.rightSupportPosition = -key.leftSupportPosition;
						key.rightSupportValue = -key.leftSupportValue;
						break;
					}
				}
			}

			info->curve->SetKeys(keys);
			info->UpdateHandles();
			info->UpdateApproximatedPoints();
		}

		CheckHandlesVisible();
		RecalculateViewArea();
		mNeedRedraw = true;
	}

	void UICurveEditor::OnTransformFrameTransformed(const Basis& basis)
	{
		Vec2F border(10, 10);
		Basis localBasis(ScreenToLocalPoint(basis.offs + border),
			(basis.xv - Vec2F(border.x*2.0f, 0))*mViewCamera.GetScale(),
						 (basis.yv - Vec2F(0, border.y*2.0f))*mViewCamera.GetScale());

		Basis lastTransformBasis = mTransformFrameBasis;
		Basis delta = mTransformFrameBasis.Inverted()*localBasis;

		if (delta.offs.Length() > 0.01f || delta.xv != Vec2F(1, 0) || delta.yv != Vec2F(0, 1))
		{
			for (auto handle : mSelectedHandles)
			{
				if (mSupportHandles.Contains(handle))
					continue;

				handle->position = handle->position*delta;
				handle->onChangedPos(handle->GetPosition());
			}

			if (o2Input.IsKeyDown(VK_CONTROL))
			{
				float right = lastTransformBasis.offs.x + lastTransformBasis.xv.x;
				float left = lastTransformBasis.offs.x;

				float rightOffset = localBasis.offs.x + localBasis.xv.x - right;
				float leftOffset = localBasis.offs.x - left;

				bool rightChanged = !Math::Equals(rightOffset, 0.0f);
				bool leftChanged = !Math::Equals(leftOffset, 0.0f);

				for (auto handle : mHandles)
				{
					if (mSelectedHandles.Contains(handle) || mSupportHandles.Contains(handle))
						continue;

					if (handle->GetPosition().x >= right && rightChanged)
					{
						handle->position += Vec2F(rightOffset, 0);
						handle->onChangedPos(handle->GetPosition());
					}

					if (handle->GetPosition().x <= left && leftChanged)
					{
						handle->position += Vec2F(leftOffset, 0);
						handle->onChangedPos(handle->GetPosition());
					}
				}
			}

			UpdateTransformFrame();
		}
	}

	void UICurveEditor::OnTransformBegin()
	{
		mBeforeTransformKeys.Clear();

		CurveKeysInfo keysInfo;
		for (auto curveInfo : mCurves)
		{
			keysInfo.keys.Clear();
			keysInfo.selectedHandles.Clear();

			for (auto handles : curveInfo->handles)
			{
				if (handles->IsSomeHandleSelected())
				{
					keysInfo.keys.Add(curveInfo->curve->GetKeyAt(handles->curveKeyIdx));

					keysInfo.selectedHandles.Add(SelectedHandlesInfo());
					keysInfo.selectedHandles.Last().index = handles->curveKeyIdx;
					keysInfo.selectedHandles.Last().mainHandle = handles->mainHandle.IsSelected();
					keysInfo.selectedHandles.Last().leftSupportHandle = handles->leftSupportHandle.IsSelected();
					keysInfo.selectedHandles.Last().rightSupportHandle = handles->rightSupportHandle.IsSelected();
				}
			}

			if (!keysInfo.keys.IsEmpty())
			{
				keysInfo.curveId = curveInfo->curveId;
				mBeforeTransformKeys.Add(keysInfo);
			}
		}
	}

	void UICurveEditor::OnTransformCompleted()
	{
		bool changed = false;
		for (auto& keysInfo : mBeforeTransformKeys)
		{
			CurveInfo* curveInfo = mCurves.FindMatch([&](CurveInfo* x) { return x->curveId == keysInfo.curveId; });
			if (!curveInfo)
				continue;

			for (auto& key : curveInfo->curve->GetKeys())
			{
				if (!keysInfo.keys.Contains(key))
				{
					changed = true;
					break;
				}
			}

			if (changed)
				break;
		}

		if (changed)
		{
			KeysChangeAction::KeysInfosVec actionKeysInfos;
			for (auto& keysInfo : mBeforeTransformKeys)
			{
				CurveInfo* curveInfo = mCurves.FindMatch([&](CurveInfo* x) { return x->curveId == keysInfo.curveId; });
				if (!curveInfo)
					continue;

				actionKeysInfos.Add(KeysChangeAction::KeysInfo());
				auto& actionCurveKeysInfo = actionKeysInfos.Last();

				actionCurveKeysInfo.curveId = keysInfo.curveId;
				actionCurveKeysInfo.beforeKeys = keysInfo.keys;
				actionCurveKeysInfo.selectedHandles = keysInfo.selectedHandles;

				for (auto handles : curveInfo->handles)
				{
					if (handles->IsSomeHandleSelected())
						actionCurveKeysInfo.afterKeys.Add(curveInfo->curve->GetKeyAt(handles->curveKeyIdx));
				}
			}

			KeysChangeAction* action = mnew KeysChangeAction(actionKeysInfos, this);
			DoneAction(action);
		}
	}

	void UICurveEditor::OnEditKeyPositionChanged(const WString& str)
	{
		for (auto handle : mSelectedHandles)
		{
			if (mSupportHandles.Contains(handle))
				continue;

			handle->SetPosition(Vec2F((float)str, handle->position->y));
			handle->onChangedPos(handle->position);
		}

		UpdateTransformFrame();
	}

	void UICurveEditor::OnEditKeyValueChanged(const WString& str)
	{
		for (auto handle : mSelectedHandles)
		{
			if (mSupportHandles.Contains(handle))
				continue;

			handle->SetPosition(Vec2F(handle->position->x, (float)str));
			handle->onChangedPos(handle->position);
		}

		UpdateTransformFrame();
	}

	void UICurveEditor::DoneAction(IAction* action)
	{
		mUndoActions.Add(action);

		for (auto action : mRedoActions)
			delete action;

		mRedoActions.Clear();
	}

	void UICurveEditor::OnEditPressed()
	{
		Vec2F key;
		bool someSelected = false;
		for (auto handle : mSelectedHandles)
		{
			if (mSupportHandles.Contains(handle))
				continue;

			someSelected = true;

			key = handle->position;
		}

		mEditValueWindowPosition->text = (WString)key.x;
		mEditValueWindowValue->text = (WString)key.y;
		mEditValueWindow->ShowModal();
	}

	void UICurveEditor::OnAutoSmoothChecked(bool checked)
	{
		mContextMenu->SetItemChecked(0, true);  // auto smooth
		mContextMenu->SetItemChecked(1, false); // flat
		mContextMenu->SetItemChecked(2, false); // free
		mContextMenu->SetItemChecked(3, false); // broken
		mContextMenu->SetItemChecked(4, false); // discrete

		SetSelectedKeysSupportsType(Curve::Key::Type::Smooth);
	}

	void UICurveEditor::OnFlatChecked(bool checked)
	{
		mContextMenu->SetItemChecked(0, false);  // auto smooth
		mContextMenu->SetItemChecked(1, true); // flat
		mContextMenu->SetItemChecked(2, false); // free
		mContextMenu->SetItemChecked(3, false); // broken
		mContextMenu->SetItemChecked(4, false); // discrete

		SetSelectedKeysSupportsType(Curve::Key::Type::Flat);
	}

	void UICurveEditor::OnFreeChecked(bool checked)
	{
		mContextMenu->SetItemChecked(0, false);  // auto smooth
		mContextMenu->SetItemChecked(1, false); // flat
		mContextMenu->SetItemChecked(2, true); // free
		mContextMenu->SetItemChecked(3, false); // broken
		mContextMenu->SetItemChecked(4, false); // discrete

		SetSelectedKeysSupportsType(Curve::Key::Type::Free);
	}

	void UICurveEditor::OnBrokenChecked(bool checked)
	{
		mContextMenu->SetItemChecked(0, false);  // auto smooth
		mContextMenu->SetItemChecked(1, false); // flat
		mContextMenu->SetItemChecked(2, false); // free
		mContextMenu->SetItemChecked(3, true); // broken
		mContextMenu->SetItemChecked(4, false); // discrete

		SetSelectedKeysSupportsType(Curve::Key::Type::Broken);
	}

	void UICurveEditor::OnDiscreteChecked(bool checked)
	{
		mContextMenu->SetItemChecked(0, false);  // auto smooth
		mContextMenu->SetItemChecked(1, false); // flat
		mContextMenu->SetItemChecked(2, false); // free
		mContextMenu->SetItemChecked(3, false); // broken
		mContextMenu->SetItemChecked(4, true); // discrete

		SetSelectedKeysSupportsType(Curve::Key::Type::Discrete);
	}

	void UICurveEditor::OnCopyPressed()
	{
		if (!IsFocused())
			return;

		CurveCopyInfosVec copyKeys;

		for (auto curve : mCurves)
		{
			CurveCopyInfo* copyInfo = mnew CurveCopyInfo();
			copyInfo->curveId = curve->curveId;

			for (auto handles : curve->handles)
			{
				if (!handles->mainHandle.IsSelected())
					continue;

				copyInfo->keys.Add(curve->curve->GetKeyAt(handles->curveKeyIdx));
			}

			if (copyInfo->keys.IsEmpty())
				delete copyInfo;
			else
				copyKeys.Add(copyInfo);
		}

		DataNode copyData;
		copyData = copyKeys;
		String copyDataStr = copyData.SaveAsWString();

		Clipboard::SetText(copyDataStr);
	}

	void UICurveEditor::OnCutPressed()
	{
		if (!IsFocused())
			return;

		OnCopyPressed();
		OnDeletePressed();
	}

	void UICurveEditor::OnPastePressed()
	{
		if (!IsFocused())
			return;

		float insertPos = ScreenToLocalPoint(o2Input.cursorPos).x;
		DataNode data;
		data.LoadFromData(Clipboard::GetText());

		CurveCopyInfosVec copyKeys;
		copyKeys = data;

		CurveKeysInfosVec keyInfos;

		for (auto curve : copyKeys)
		{
			CurveInfo* curveInfo = mCurves.FindMatch([=](const CurveInfo* x) { return x->curveId == curve->curveId; });

			if (curveInfo == nullptr)
			{
				if (copyKeys.Count() == 1 && mCurves.Count() == 1)
					curveInfo = mCurves[0];
			}

			if (!curveInfo)
				continue;

			keyInfos.Add(CurveKeysInfo());
			keyInfos.Last().curveId = curveInfo->curveId;

			float positionDelta = insertPos - curve->keys[0].position;
			for (auto& key : curve->keys)
			{
				key.position += positionDelta;

				int position = curveInfo->curve->InsertKey(key);
				AddCurveKeyHandles(curveInfo, position);

				keyInfos.Last().keys.Add(key);
			}

			delete curve;

			curveInfo->UpdateApproximatedPoints();
			curveInfo->UpdateHandles();
			mNeedRedraw = true;
		}

		if (!keyInfos.IsEmpty())
		{
			AddKeysAction* action = mnew AddKeysAction(keyInfos, this);
			DoneAction(action);
		}
	}

	void UICurveEditor::OnDeletePressed()
	{
		if (!IsFocused())
			return;

		CurveKeysInfosVec keyInfos;

		for (auto curveInfo : mCurves)
		{
			CurveCopyInfo* copyInfo = mnew CurveCopyInfo();
			copyInfo->curveId = curveInfo->curveId;

			Vector<int> removingIdxs;
			for (auto handles : curveInfo->handles)
			{
				if (!handles->mainHandle.IsSelected())
					continue;

				removingIdxs.Add(handles->curveKeyIdx);
			}

			if (!removingIdxs.IsEmpty())
			{
				keyInfos.Add(CurveKeysInfo());
				keyInfos.Last().curveId = curveInfo->curveId;
			}

			for (int i = removingIdxs.Count() - 1; i >= 0; i--)
			{
				keyInfos.Last().keys.Add(curveInfo->curve->GetKeyAt(removingIdxs[i]));

				curveInfo->curve->RemoveKeyAt(removingIdxs[i]);
				RemoveCurveKeyHandles(curveInfo, removingIdxs[i]);
			}

			curveInfo->UpdateApproximatedPoints();
			curveInfo->UpdateHandles();
			mNeedRedraw = true;
		}

		mSelectedHandles.Clear();
		UpdateTransformFrame();

		if (!keyInfos.IsEmpty())
		{
			DeleteKeysAction* action = mnew DeleteKeysAction(keyInfos, this);
			DoneAction(action);
		}
	}

	void UICurveEditor::OnInsertPressed()
	{
		if (!IsFocused())
			return;

		float insertPos = ScreenToLocalPoint(o2Input.cursorPos).x;
		DataNode data;
		data.LoadFromData(Clipboard::GetText());

		CurveCopyInfosVec copyKeys;
		copyKeys = data;

		for (auto curve : copyKeys)
		{
			CurveInfo* curveInfo = mCurves.FindMatch([=](const CurveInfo* x) { return x->curveId == curve->curveId; });

			if (curveInfo == nullptr)
			{
				if (copyKeys.Count() == 1 && mCurves.Count() == 1)
					curveInfo = mCurves[0];
			}

			if (!curveInfo)
				continue;

			float length = curve->keys.Last().position - curve->keys[0].position;
			curveInfo->curve->MoveKeysFrom(insertPos, length);

			float positionDelta = insertPos - curve->keys[0].position;
			for (auto& key : curve->keys)
			{
				key.position += positionDelta;

				int position = curveInfo->curve->InsertKey(key);
				AddCurveKeyHandles(curveInfo, position);
			}

			delete curve;
		}
	}

	void UICurveEditor::OnUndoPressed()
	{
		if (mUndoActions.IsEmpty())
			return;

		IAction* action = mUndoActions.PopBack();
		action->Undo();

		mRedoActions.Add(action);
	}

	void UICurveEditor::OnRedoPressed()
	{
		if (mRedoActions.IsEmpty())
			return;

		IAction* action = mRedoActions.PopBack();
		action->Redo();

		mUndoActions.Add(action);
	}

	UICurveEditor::CurveInfo::CurveInfo()
	{}

	UICurveEditor::CurveInfo::~CurveInfo()
	{
		for (auto x : handles)
		{
			x->mainHandle.SetSelectionGroup(nullptr);
			x->leftSupportHandle.SetSelectionGroup(nullptr);
			x->rightSupportHandle.SetSelectionGroup(nullptr);

			delete x;
		}
	}

	void UICurveEditor::CurveInfo::UpdateHandles()
	{
		for (int i = 0; i < handles.Count(); i++)
		{
			Curve::Key key = curve->GetKeyAt(i);
			handles[i]->mainHandle.position = Vec2F(key.position, key.value);

			if (i > 0)
			{
				handles[i]->leftSupportHandle.position = Vec2F(key.position + key.leftSupportPosition,
															   key.value + key.leftSupportValue);

				handles[i]->leftSupportHandle.enabled = handles[i]->leftSupportHandle.IsSelected() ||
					handles[i]->mainHandle.IsSelected();
			}
			else handles[i]->leftSupportHandle.enabled = false;

			if (i < handles.Count() - 1)
			{
				handles[i]->rightSupportHandle.position = Vec2F(key.position + key.rightSupportPosition,
																key.value + key.rightSupportValue);

				handles[i]->rightSupportHandle.enabled = handles[i]->rightSupportHandle.IsSelected() ||
					handles[i]->mainHandle.IsSelected();
			}
			else handles[i]->rightSupportHandle.enabled = false;
		}
	}

	void UICurveEditor::CurveInfo::UpdateApproximatedPoints()
	{
		approximatedPoints.Clear();

		auto keys = curve->GetKeys();
		for (int i = 1; i < keys.Count(); i++)
		{
			const Vec2F* keyPoints = keys[i].GetApproximatedPoints();
			for (int i = 0; i < keys[i].GetApproximatedPointsCount(); i++)
				approximatedPoints.Add(keyPoints[i]);
		}
	}

	UICurveEditor::RangeInfo::RangeInfo()
	{

	}

	UICurveEditor::RangeInfo::~RangeInfo()
	{

	}

	void UICurveEditor::RangeInfo::UpdateMesh()
	{

	}

	UICurveEditor::KeyHandles::KeyHandles(const SelectableDragHandle& mainSample, const SelectableDragHandle& supportSample,
										  UICurveEditor* editor):
		mainHandle(mainSample), leftSupportHandle(supportSample), rightSupportHandle(supportSample), curveEditor(editor)
	{}

	void UICurveEditor::KeyHandles::Draw(const RectF& camRect)
	{
		bool selected = true;

		Vec2F mainHandleScreenPos = curveEditor->LocalToScreenPoint(mainHandle.GetPosition());

		if (leftSupportHandle.enabled)
		{
			Vec2F leftSupportHandleScreenPos = curveEditor->LocalToScreenPoint(leftSupportHandle.GetPosition());
			o2Render.DrawLine(mainHandleScreenPos, leftSupportHandleScreenPos, curveEditor->mGridColor);
		}

		if (rightSupportHandle.enabled)
		{
			Vec2F rightSupportHandleScreenPos = curveEditor->LocalToScreenPoint(rightSupportHandle.GetPosition());
			o2Render.DrawLine(mainHandleScreenPos, rightSupportHandleScreenPos, curveEditor->mGridColor);
		}

		if (camRect.IsInside(mainHandle.GetPosition()))
			mainHandle.Draw();

		if (camRect.IsInside(leftSupportHandle.GetPosition()))
			leftSupportHandle.Draw();

		if (camRect.IsInside(rightSupportHandle.GetPosition()))
			rightSupportHandle.Draw();
	}

	bool UICurveEditor::KeyHandles::IsSomeHandleSelected() const
	{
		return mainHandle.IsSelected() || leftSupportHandle.IsSelected() || rightSupportHandle.IsSelected();
	}

	UICurveEditor::AddKeysAction::AddKeysAction(const CurveKeysInfosVec& infos, UICurveEditor* editor):
		mInfos(infos), mEditor(editor)
	{
		for (auto& info : mInfos)
		{
			info.keys.Sort([](const Curve::Key& a, const Curve::Key& b) { return a.position < b.position; });
		}
	}

	UICurveEditor::AddKeysAction::AddKeysAction()
	{}

	String UICurveEditor::AddKeysAction::GetName()
	{
		return "add keys";
	}

	void UICurveEditor::AddKeysAction::Redo()
	{
		mEditor->DeselectAll();
		mEditor->mSelectedHandles.Clear();

		for (auto& info : mInfos)
		{
			auto curveInfo =
				mEditor->mCurves.FindMatch([&](UICurveEditor::CurveInfo* x) { return x->curveId == info.curveId; });

			if (!curveInfo)
				continue;

			Vector<int> indexes;

			for (auto& key : info.keys)
				indexes.Add(curveInfo->curve->InsertKey(key));

			curveInfo->UpdateApproximatedPoints();
			curveInfo->UpdateHandles();

			for (int idx : indexes)
				curveInfo->handles[idx]->mainHandle.SetSelected(true);

			mEditor->mNeedRedraw = true;
		}

		mEditor->UpdateTransformFrame();
		mEditor->CheckHandlesVisible();
	}

	void UICurveEditor::AddKeysAction::Undo()
	{
		for (auto& info : mInfos)
		{
			auto curveInfo =
				mEditor->mCurves.FindMatch([&](UICurveEditor::CurveInfo* x) { return x->curveId == info.curveId; });

			if (!curveInfo)
				continue;

			for (auto& key : info.keys)
				curveInfo->curve->RemoveKey(key.position);

			curveInfo->UpdateApproximatedPoints();
			curveInfo->UpdateHandles();
			mEditor->mNeedRedraw = true;
		}

		mEditor->DeselectAll();
		mEditor->mSelectedHandles.Clear();
		mEditor->UpdateTransformFrame();
		mEditor->CheckHandlesVisible();
	}

	UICurveEditor::DeleteKeysAction::DeleteKeysAction(const CurveKeysInfosVec& infos, UICurveEditor* editor):
		mInfos(infos), mEditor(editor)
	{
		for (auto& info : mInfos)
		{
			info.keys.Sort([](const Curve::Key& a, const Curve::Key& b) { return a.position < b.position; });
		}
	}

	UICurveEditor::DeleteKeysAction::DeleteKeysAction()
	{}

	String UICurveEditor::DeleteKeysAction::GetName()
	{
		return "remove keys";
	}

	void UICurveEditor::DeleteKeysAction::Redo()
	{
		for (auto& info : mInfos)
		{
			auto curveInfo =
				mEditor->mCurves.FindMatch([&](UICurveEditor::CurveInfo* x) { return x->curveId == info.curveId; });

			if (!curveInfo)
				continue;

			for (auto& key : info.keys)
				curveInfo->curve->RemoveKey(key.position);

			curveInfo->UpdateApproximatedPoints();
			curveInfo->UpdateHandles();
			mEditor->mNeedRedraw = true;
		}

		mEditor->DeselectAll();
		mEditor->mSelectedHandles.Clear();
		mEditor->UpdateTransformFrame();
		mEditor->CheckHandlesVisible();
	}

	void UICurveEditor::DeleteKeysAction::Undo()
	{
		mEditor->DeselectAll();
		mEditor->mSelectedHandles.Clear();

		for (auto& info : mInfos)
		{
			auto curveInfo =
				mEditor->mCurves.FindMatch([&](UICurveEditor::CurveInfo* x) { return x->curveId == info.curveId; });

			if (!curveInfo)
				continue;

			Vector<int> indexes;

			for (auto& key : info.keys)
				indexes.Add(curveInfo->curve->InsertKey(key));

			curveInfo->UpdateApproximatedPoints();
			curveInfo->UpdateHandles();

			for (int idx : indexes)
				curveInfo->handles[idx]->mainHandle.SetSelected(true);

			mEditor->mNeedRedraw = true;
		}

		mEditor->UpdateTransformFrame();
		mEditor->CheckHandlesVisible();
	}

	UICurveEditor::KeysChangeAction::KeysChangeAction(const KeysInfosVec& infos, UICurveEditor* editor):
		mInfos(infos), mEditor(editor)
	{}

	UICurveEditor::KeysChangeAction::KeysChangeAction()
	{}

	String UICurveEditor::KeysChangeAction::GetName()
	{
		return "change keys";
	}

	void UICurveEditor::KeysChangeAction::Redo()
	{
		mEditor->DeselectAll();
		mEditor->mSelectedHandles.Clear();

		for (auto& info : mInfos)
		{
			auto curveInfo =
				mEditor->mCurves.FindMatch([&](UICurveEditor::CurveInfo* x) { return x->curveId == info.curveId; });

			if (!curveInfo)
				continue;

			for (auto& key : info.beforeKeys)
				curveInfo->curve->RemoveKey(key.position);

			for (auto& key : info.afterKeys)
				curveInfo->curve->InsertKey(key);

			for (auto& selectedHandles : info.selectedHandles)
			{
				auto handles = curveInfo->handles[selectedHandles.index];

				if (selectedHandles.mainHandle)
					handles->mainHandle.Select();

				if (selectedHandles.leftSupportHandle)
					handles->leftSupportHandle.Select();

				if (selectedHandles.rightSupportHandle)
					handles->rightSupportHandle.Select();
			}

			curveInfo->UpdateApproximatedPoints();
			curveInfo->UpdateHandles();
			mEditor->mNeedRedraw = true;
		}

		mEditor->UpdateTransformFrame();
		mEditor->CheckHandlesVisible();
	}

	void UICurveEditor::KeysChangeAction::Undo()
	{
		mEditor->DeselectAll();
		mEditor->mSelectedHandles.Clear();

		for (auto& info : mInfos)
		{
			auto curveInfo =
				mEditor->mCurves.FindMatch([&](UICurveEditor::CurveInfo* x) { return x->curveId == info.curveId; });

			if (!curveInfo)
				continue;

			for (auto& key : info.afterKeys)
				curveInfo->curve->RemoveKey(key.position);

			for (auto& key : info.beforeKeys)
				curveInfo->curve->InsertKey(key);

			for (auto& selectedHandles : info.selectedHandles)
			{
				auto handles = curveInfo->handles[selectedHandles.index];

				if (selectedHandles.mainHandle)
					handles->mainHandle.Select();

				if (selectedHandles.leftSupportHandle)
					handles->leftSupportHandle.Select();

				if (selectedHandles.rightSupportHandle)
					handles->rightSupportHandle.Select();
			}

			curveInfo->UpdateApproximatedPoints();
			curveInfo->UpdateHandles();
			mEditor->mNeedRedraw = true;
		}

		mEditor->UpdateTransformFrame();
		mEditor->CheckHandlesVisible();
	}

	bool UICurveEditor::CurveKeysInfo::operator==(const CurveKeysInfo& other) const
	{
		return curveId == other.curveId && keys == other.keys;
	}

	bool UICurveEditor::KeysChangeAction::KeysInfo::operator==(const KeysInfo& other) const
	{
		return curveId == other.curveId && beforeKeys == other.beforeKeys && afterKeys == other.afterKeys;
	}

	bool UICurveEditor::SelectedHandlesInfo::operator==(const SelectedHandlesInfo& other) const
	{
		return index == other.index && mainHandle == other.mainHandle && leftSupportHandle == other.leftSupportHandle &&
			rightSupportHandle == other.rightSupportHandle;
	}

}

CLASS_META(Editor::UICurveEditor)
{
	BASE_CLASS(Editor::UIFrameScrollView);
	BASE_CLASS(o2::SelectableDragHandlesGroup);

	PROTECTED_FIELD(mContextMenu);
	PROTECTED_FIELD(mMainHandleSample).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mSupportHandleSample).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mCurves);
	PROTECTED_FIELD(mRanges);
	PROTECTED_FIELD(mSupportHandles);
	PROTECTED_FIELD(mSelectingHandlesBuf);
	PROTECTED_FIELD(mSelectionSprite).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mTextFont).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mTextLeft);
	PROTECTED_FIELD(mTextRight);
	PROTECTED_FIELD(mTextTop);
	PROTECTED_FIELD(mTextBottom);
	PROTECTED_FIELD(mSelectingPressedPoint);
	PROTECTED_FIELD(mTransformFrame);
	PROTECTED_FIELD(mTransformFrameVisible);
	PROTECTED_FIELD(mTransformFrameBasis);
	PROTECTED_FIELD(mIsViewScrolling);
	PROTECTED_FIELD(mEditValueWindow);
	PROTECTED_FIELD(mEditValueWindowValue);
	PROTECTED_FIELD(mEditValueWindowPosition);
	PROTECTED_FIELD(mBeforeTransformKeys);
	PROTECTED_FIELD(mUndoActions);
	PROTECTED_FIELD(mRedoActions);

	PUBLIC_FUNCTION(void, Draw);
	PUBLIC_FUNCTION(void, Update, float);
	PUBLIC_FUNCTION(void, AddEditingCurve, const String&, Curve*, const Color4&);
	PUBLIC_FUNCTION(void, RemoveEditingCurve, Curve*);
	PUBLIC_FUNCTION(void, RemoveEditingCurve, const String&);
	PUBLIC_FUNCTION(void, RemoveAllEditingCurves);
	PUBLIC_FUNCTION(void, AddCurvesRange, Curve*, Curve*, const Color4&);
	PUBLIC_FUNCTION(void, RemoveCurvesRange, Curve*, Curve*);
	PUBLIC_FUNCTION(void, AddCurvesRange, const String&, const String&, const Color4&);
	PUBLIC_FUNCTION(void, RemoveCurvesRange, const String&, const String&);
	PUBLIC_FUNCTION(void, SetSelectionSpriteImage, const ImageAssetRef&);
	PUBLIC_FUNCTION(void, SetTextFont, const FontRef&);
	PUBLIC_FUNCTION(void, SetMainHandleImages, const ImageAssetRef&, const ImageAssetRef&, const ImageAssetRef&, const ImageAssetRef&);
	PUBLIC_FUNCTION(void, SetSupportHandleImages, const ImageAssetRef&, const ImageAssetRef&, const ImageAssetRef&, const ImageAssetRef&);
	PUBLIC_FUNCTION(void, UpdateLayout, bool, bool);
	PROTECTED_FUNCTION(void, OnVisibleChanged);
	PROTECTED_FUNCTION(void, OnScrolled, float);
	PROTECTED_FUNCTION(Curve*, FindCurve, const String&);
	PROTECTED_FUNCTION(void, InitializeContextMenu);
	PROTECTED_FUNCTION(void, InitializeTextDrawables);
	PROTECTED_FUNCTION(void, InitializeEditValueWindow);
	PROTECTED_FUNCTION(void, RecalculateViewArea);
	PROTECTED_FUNCTION(void, RedrawContent);
	PROTECTED_FUNCTION(void, DrawGrid);
	PROTECTED_FUNCTION(void, DrawCurves);
	PROTECTED_FUNCTION(void, DrawHandles);
	PROTECTED_FUNCTION(void, DrawSelection);
	PROTECTED_FUNCTION(void, DrawTransformFrame);
	PROTECTED_FUNCTION(void, AddCurveKeyHandles, CurveInfo*, int);
	PROTECTED_FUNCTION(void, RemoveCurveKeyHandles, CurveInfo*, int);
	PROTECTED_FUNCTION(void, OnCurveKeyMainHandleDragged, CurveInfo*, KeyHandles*, const Vec2F&);
	PROTECTED_FUNCTION(void, OnCurveKeyLeftSupportHandleDragged, CurveInfo*, KeyHandles*, const Vec2F&);
	PROTECTED_FUNCTION(void, OnCurveKeyRightSupportHandleDragged, CurveInfo*, KeyHandles*, const Vec2F&);
	PROTECTED_FUNCTION(Vec2F, CheckLeftSupportHandlePosition, CurveInfo*, KeyHandles*, const Vec2F&);
	PROTECTED_FUNCTION(Vec2F, CheckRightSupportHandlePosition, CurveInfo*, KeyHandles*, const Vec2F&);
	PROTECTED_FUNCTION(void, OnCursorDblClicked, const Input::Cursor&);
	PROTECTED_FUNCTION(void, SmoothKey, CurveInfo*, int);
	PROTECTED_FUNCTION(void, OnCursorPressed, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorReleased, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorStillDown, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorRightMouseStayDown, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnCursorRightMouseReleased, const Input::Cursor&);
	PROTECTED_FUNCTION(void, CheckHandlesVisible);
	PROTECTED_FUNCTION(void, UpdateTransformFrame);
	PROTECTED_FUNCTION(bool, IsTransformFrameVisible);
	PROTECTED_FUNCTION(void, OnHandleCursorReleased, SelectableDragHandle*, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnHandleBeganDragging, SelectableDragHandle*);
	PROTECTED_FUNCTION(void, OnHandleMoved, SelectableDragHandle*, const Input::Cursor&);
	PROTECTED_FUNCTION(void, OnHandleCompletedChange, SelectableDragHandle*);
	PROTECTED_FUNCTION(void, SetSelectedKeysSupportsType, Curve::Key::Type);
	PROTECTED_FUNCTION(void, OnTransformFrameTransformed, const Basis&);
	PROTECTED_FUNCTION(void, OnTransformBegin);
	PROTECTED_FUNCTION(void, OnTransformCompleted);
	PROTECTED_FUNCTION(void, OnEditKeyPositionChanged, const WString&);
	PROTECTED_FUNCTION(void, OnEditKeyValueChanged, const WString&);
	PROTECTED_FUNCTION(void, DoneAction, IAction*);
	PROTECTED_FUNCTION(void, OnEditPressed);
	PROTECTED_FUNCTION(void, OnAutoSmoothChecked, bool);
	PROTECTED_FUNCTION(void, OnFlatChecked, bool);
	PROTECTED_FUNCTION(void, OnFreeChecked, bool);
	PROTECTED_FUNCTION(void, OnBrokenChecked, bool);
	PROTECTED_FUNCTION(void, OnDiscreteChecked, bool);
	PROTECTED_FUNCTION(void, OnCopyPressed);
	PROTECTED_FUNCTION(void, OnCutPressed);
	PROTECTED_FUNCTION(void, OnPastePressed);
	PROTECTED_FUNCTION(void, OnDeletePressed);
	PROTECTED_FUNCTION(void, OnInsertPressed);
	PROTECTED_FUNCTION(void, OnUndoPressed);
	PROTECTED_FUNCTION(void, OnRedoPressed);
}
END_META;

CLASS_META(Editor::UICurveEditor::CurveCopyInfo)
{
	BASE_CLASS(o2::ISerializable);

	PUBLIC_FIELD(curveId).SERIALIZABLE_ATTRIBUTE();
	PUBLIC_FIELD(keys).SERIALIZABLE_ATTRIBUTE();
}
END_META;

CLASS_META(Editor::UICurveEditor::AddKeysAction)
{
	BASE_CLASS(Editor::IAction);

	PROTECTED_FIELD(mInfos);
	PROTECTED_FIELD(mEditor);

	PUBLIC_FUNCTION(String, GetName);
	PUBLIC_FUNCTION(void, Redo);
	PUBLIC_FUNCTION(void, Undo);
}
END_META;

CLASS_META(Editor::UICurveEditor::DeleteKeysAction)
{
	BASE_CLASS(Editor::IAction);

	PROTECTED_FIELD(mInfos);
	PROTECTED_FIELD(mEditor);

	PUBLIC_FUNCTION(String, GetName);
	PUBLIC_FUNCTION(void, Redo);
	PUBLIC_FUNCTION(void, Undo);
}
END_META;

CLASS_META(Editor::UICurveEditor::KeysChangeAction)
{
	BASE_CLASS(Editor::IAction);

	PROTECTED_FIELD(mInfos);
	PROTECTED_FIELD(mEditor);

	PUBLIC_FUNCTION(String, GetName);
	PUBLIC_FUNCTION(void, Redo);
	PUBLIC_FUNCTION(void, Undo);
}
END_META;
