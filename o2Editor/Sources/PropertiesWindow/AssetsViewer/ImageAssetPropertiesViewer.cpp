#include "ImageAssetPropertiesViewer.h"

#include "Assets/ImageAsset.h"
#include "PropertiesWindow/Properties/BorderIntProperty.h"
#include "PropertiesWindow/Properties/EnumProperty.h"
#include "PropertiesWindow/Properties/ObjectProperty.h"
#include "PropertiesWindow/Properties/RectangleIntProperty.h"
#include "PropertiesWindow/PropertiesWindow.h"
#include "UI/Image.h"
#include "UI/Label.h"
#include "UI/UIManager.h"
#include "UI/VerticalLayout.h"
#include "UI/Widget.h"
#include "Utils/Reflection/Reflection.h"

namespace Editor
{
	ImageAssetPropertiesViewer::ImageAssetPropertiesViewer()
	{
		mContent = mnew UIVerticalLayout();
		mContent->spacing = 5.0f;
		mContent->border = RectF(5, 5, 12, 5);
		mContent->expandHeight = false;
		mContent->expandWidth = true;
		mContent->fitByChildren = true;
		mContent->baseCorner = BaseCorner::Top;

		InitializeImagePreview();
		InitializeProperties();

		mContent->UpdateLayout(true);
	}

	ImageAssetPropertiesViewer::~ImageAssetPropertiesViewer()
	{
		delete mContent;
	}

	void ImageAssetPropertiesViewer::SetTargetAssets(const Vector<Asset*>& assets)
	{
		mTargetAssets = assets.Cast<ImageAsset*>();

		auto borderTargets = mTargetAssets.Select<void*>([](const ImageAsset* x) { return &(x->GetMeta()->mSliceBorder); });
		mBorderProperty->Setup(borderTargets, false);

		auto modeTargets = mTargetAssets.Select<void*>([](const ImageAsset* x) { return &(x->GetMeta()->mDefaultMode); });
		mDefaultTypeProperty->Setup(modeTargets, false);

		auto windowsTargets = mTargetAssets.Select<void*>([](const ImageAsset* x) { return &(x->GetMeta()->mWindows); });
		mWindowsProperties->Setup(windowsTargets, false);

		auto osxTargets = mTargetAssets.Select<void*>([](const ImageAsset* x) { return &(x->GetMeta()->mMacOS); });
		mOSXProperties->Setup(osxTargets, false);

		auto androidTargets = mTargetAssets.Select<void*>([](const ImageAsset* x) { return &(x->GetMeta()->mAndroid); });
		mAndroidProperties->Setup(androidTargets, false);

		auto iosTargets = mTargetAssets.Select<void*>([](const ImageAsset* x) { return &(x->GetMeta()->mIOS); });
		mIOSProperties->Setup(iosTargets, false);

		mPreviewImage->imageAsset = *mTargetAssets.Last();
		mPreviewImage->GetImage()->mode = SpriteMode::Default;

		FitImage();
		SetupAtlasProperty();
	}

	const Type* ImageAssetPropertiesViewer::GetAssetType() const
	{
		return &TypeOf(ImageAsset);
	}

	UIWidget* ImageAssetPropertiesViewer::GetWidget() const
	{
		return mContent;
	}

	void ImageAssetPropertiesViewer::InitializeImagePreview()
	{
		mPreviewImageContent = mnew UIWidget();
		mPreviewImageContent->layout.minHeight = 200;

		auto separatorImg = o2UI.CreateImage("ui/UI_Separator.png");
		separatorImg->layout = UIWidgetLayout::HorStretch(VerAlign::Bottom, -6, -15, 5, -4);
		mPreviewImageContent->AddChild(separatorImg);

		mPreviewImageBack = mnew UIImage();
		mPreviewImageBack->layout = UIWidgetLayout::BothStretch();
		mPreviewImageBack->GetImage()->color = Color4(100, 100, 100, 255);
		mPreviewImageContent->AddChild(mPreviewImageBack);

		mPreviewImage = mnew UIImage();
		mPreviewImage->layout = UIWidgetLayout::BothStretch();
		mPreviewImageContent->AddChild(mPreviewImage);

		InitializeLeftHandle();
		InitializeRightHandle();
		InitializeTopHandle();
		InitializeBottomHandle();

		mContent->AddChild(mPreviewImageContent);
	}

	void ImageAssetPropertiesViewer::InitializeLeftHandle()
	{
		mBorderLeftHandleWidget = mnew UIImage();
		mBorderLeftHandleWidget->SetImage(mnew Sprite(Color4::Green()));
		mPreviewImage->AddChild(mBorderLeftHandleWidget);

		mBorderLeftHandle.cursorType = CursorType::SizeWE;
		mBorderLeftHandleWidget->onDraw += [&]() { mBorderLeftHandle.OnDrawn(); };
		mBorderLeftHandle.isUnderPoint = [&](const Vec2F& p) {
			auto rt = mBorderLeftHandleWidget->layout.GetAbsoluteRect();
			rt.left -= 2; rt.right += 2;
			return rt.IsInside(p);
		};

		mBorderLeftHandle.onMoved = [&](const Input::Cursor& cursor) {
			float px = mPreviewImage->layout.GetWidth()/mPreviewImage->GetImage()->GetOriginalSize().x;
			mBordersSmoothValue.left += cursor.delta.x/px;

			if (mBorderProperty->GetCommonValue().left != Math::FloorToInt(mBordersSmoothValue.left))
				UpdateBordersValue();
		};
	}

	void ImageAssetPropertiesViewer::InitializeRightHandle()
	{
		mBorderRightHandleWidget = mnew UIImage();
		mBorderRightHandleWidget->SetImage(mnew Sprite(Color4::Green()));
		mPreviewImage->AddChild(mBorderRightHandleWidget);

		mBorderRightHandle.cursorType = CursorType::SizeWE;
		mBorderRightHandleWidget->onDraw += [&]() { mBorderRightHandle.OnDrawn(); };
		mBorderRightHandle.isUnderPoint = [&](const Vec2F& p) {
			auto rt = mBorderRightHandleWidget->layout.GetAbsoluteRect();
			rt.left -= 2; rt.right += 2;
			return rt.IsInside(p);
		};

		mBorderRightHandle.onMoved = [&](const Input::Cursor& cursor) {
			float px = mPreviewImage->layout.GetWidth()/mPreviewImage->GetImage()->GetOriginalSize().x;
			mBordersSmoothValue.right -= cursor.delta.x/px;

			if (mBorderProperty->GetCommonValue().right != Math::FloorToInt(mBordersSmoothValue.right))
				UpdateBordersValue();
		};
	}

	void ImageAssetPropertiesViewer::InitializeTopHandle()
	{
		mBorderTopHandleWidget = mnew UIImage();
		mBorderTopHandleWidget->SetImage(mnew Sprite(Color4::Green()));
		mPreviewImage->AddChild(mBorderTopHandleWidget);

		mBorderTopHandle.cursorType = CursorType::SizeNS;
		mBorderTopHandleWidget->onDraw += [&]() { mBorderTopHandle.OnDrawn(); };
		mBorderTopHandle.isUnderPoint = [&](const Vec2F& p) {
			auto rt = mBorderBottomHandleWidget->layout.GetAbsoluteRect();
			rt.bottom -= 2; rt.top += 2;
			return rt.IsInside(p);
		};

		mBorderTopHandle.onMoved = [&](const Input::Cursor& cursor) {
			float px = mPreviewImage->layout.GetHeight()/mPreviewImage->GetImage()->GetOriginalSize().y;
			mBordersSmoothValue.top += cursor.delta.y/px;

			if (mBorderProperty->GetCommonValue().top != Math::FloorToInt(mBordersSmoothValue.top))
				UpdateBordersValue();
		};
	}

	void ImageAssetPropertiesViewer::InitializeBottomHandle()
	{
		mBorderBottomHandleWidget = mnew UIImage();
		mBorderBottomHandleWidget->SetImage(mnew Sprite(Color4::Green()));
		mPreviewImage->AddChild(mBorderBottomHandleWidget);

		mBorderBottomHandle.cursorType = CursorType::SizeNS;
		mBorderBottomHandleWidget->onDraw += [&]() { mBorderBottomHandle.OnDrawn(); };
		mBorderBottomHandle.isUnderPoint = [&](const Vec2F& p) {
			auto rt = mBorderTopHandleWidget->layout.GetAbsoluteRect();
			rt.bottom -= 2; rt.top += 2;
			return rt.IsInside(p);
		};

		mBorderBottomHandle.onMoved = [&](const Input::Cursor& cursor) {
			float px = mPreviewImage->layout.GetHeight()/mPreviewImage->GetImage()->GetOriginalSize().y;
			mBordersSmoothValue.bottom -= cursor.delta.y/px;

			if (mBorderProperty->GetCommonValue().bottom != Math::FloorToInt(mBordersSmoothValue.bottom))
				UpdateBordersValue();
		};
	}

	void ImageAssetPropertiesViewer::InitializeProperties()
	{
		auto borderPropertyPair = o2EditorProperties.CreateFieldProperty(&TypeOf(RectI));
		auto nameLabel = borderPropertyPair.mSecond->FindChild<UILabel>();
		nameLabel->text = "Slice border";
		mBorderProperty = (BorderIProperty*)borderPropertyPair.mFirst;
		mBorderProperty->onChanged += [&]() { UpdateBordersAnchors(); mBordersSmoothValue = mBorderProperty->GetCommonValue(); };
		mContent->AddChild(borderPropertyPair.mSecond);

		auto modePropertyPair = o2EditorProperties.CreateFieldProperty(&TypeOf(SpriteMode));
		nameLabel = modePropertyPair.mSecond->FindChild<UILabel>();
		nameLabel->text = "Default mode";
		mDefaultTypeProperty = (EnumProperty*)modePropertyPair.mFirst;
		mDefaultTypeProperty->SpecializeType(&TypeOf(SpriteMode));
		mContent->AddChild(modePropertyPair.mSecond);

		auto atlasPropertyPair = o2EditorProperties.CreateFieldProperty(&TypeOf(AtlasAsset));
		nameLabel = atlasPropertyPair.mSecond->FindChild<UILabel>();
		nameLabel->text = "Atlas";
		mAtlasProperty = (AssetProperty<AtlasAsset>*)atlasPropertyPair.mFirst;
		mAtlasProperty->onChanged = Function<void()>(this, &ImageAssetPropertiesViewer::OnAtlasPropertyChanged);
		mContent->AddChild(atlasPropertyPair.mSecond);

		auto windowsPropertyPair = o2EditorProperties.CreateFieldProperty(&TypeOf(ImageAsset::PlatformMeta));
		nameLabel = windowsPropertyPair.mSecond->FindChild<UILabel>();
		nameLabel->text = "Windows";
		mWindowsProperties = (ObjectProperty*)windowsPropertyPair.mFirst;
		mWindowsProperties->SpecializeType(&TypeOf(ImageAsset::PlatformMeta));
		mContent->AddChild(windowsPropertyPair.mSecond);

		auto osxPropertyPair = o2EditorProperties.CreateFieldProperty(&TypeOf(ImageAsset::PlatformMeta));
		nameLabel = osxPropertyPair.mSecond->FindChild<UILabel>();
		nameLabel->text = "OSX";
		mOSXProperties = (ObjectProperty*)osxPropertyPair.mFirst;
		mOSXProperties->SpecializeType(&TypeOf(ImageAsset::PlatformMeta));
		mContent->AddChild(osxPropertyPair.mSecond);

		auto androidPropertyPair = o2EditorProperties.CreateFieldProperty(&TypeOf(ImageAsset::PlatformMeta));
		nameLabel = androidPropertyPair.mSecond->FindChild<UILabel>();
		nameLabel->text = "Android";
		mAndroidProperties = (ObjectProperty*)androidPropertyPair.mFirst;
		mAndroidProperties->SpecializeType(&TypeOf(ImageAsset::PlatformMeta));
		mContent->AddChild(androidPropertyPair.mSecond);

		auto iosPropertyPair = o2EditorProperties.CreateFieldProperty(&TypeOf(ImageAsset::PlatformMeta));
		nameLabel = iosPropertyPair.mSecond->FindChild<UILabel>();
		nameLabel->text = "iOS";
		mIOSProperties = (ObjectProperty*)iosPropertyPair.mFirst;
		mIOSProperties->SpecializeType(&TypeOf(ImageAsset::PlatformMeta));
		mContent->AddChild(iosPropertyPair.mSecond);
	}

	void ImageAssetPropertiesViewer::FitImage()
	{
		Vec2F maxSize = mPreviewImageContent->layout.size;
		Vec2F imageSize = mPreviewImage->GetImage()->GetOriginalSize();

		float heightFitScale = maxSize.y/imageSize.y;
		float widthFitDifference = maxSize.x - imageSize.x*heightFitScale;

		float widthFitScale = maxSize.x/imageSize.x;
		float heightFitDifference = maxSize.y - imageSize.y*widthFitScale;

		bool fitByHeight = true;
		if (widthFitDifference > 0.0f && heightFitDifference > 0.0f)
			fitByHeight = heightFitDifference < widthFitDifference;
		else if (widthFitDifference > 0.0f)
			fitByHeight = true;
		else
			fitByHeight = false;

		float imageSizeAspect = imageSize.x/imageSize.y;
		float contentAspect = maxSize.x/maxSize.y;

		if (fitByHeight)
		{
			float d = imageSizeAspect*0.5f/contentAspect;
			mPreviewImage->layout = UIWidgetLayout(Vec2F(0.5f - d, 0.0f), Vec2F(0.5f + d, 1.0f), Vec2F(), Vec2F());
		}
		else
		{
			float d = 1.0f/imageSizeAspect*0.5f*contentAspect;
			mPreviewImage->layout = UIWidgetLayout(Vec2F(0.0f, 0.5f - d), Vec2F(1.0f, 0.5f + d), Vec2F(), Vec2F());
		}

		mPreviewImageBack->layout = mPreviewImage->layout;

		UpdateBordersAnchors();
		mBordersSmoothValue = mBorderProperty->GetCommonValue();
	}

	void ImageAssetPropertiesViewer::UpdateBordersAnchors()
	{
		Vec2F imageSize = mPreviewImage->GetImage()->GetOriginalSize();
		BorderI borders = mBorderProperty->GetCommonValue();
		BorderF bordersAnchors((float)borders.left/imageSize.x, (float)borders.top/imageSize.y,
							 1.0f - (float)borders.right/imageSize.x, 1.0f - (float)borders.bottom/imageSize.y);

		mBorderLeftHandleWidget->layout = UIWidgetLayout(Vec2F(bordersAnchors.left, 0.0f), Vec2F(bordersAnchors.left, 1.0f), Vec2F(0, 0), Vec2F(1, 0));
		mBorderRightHandleWidget->layout = UIWidgetLayout(Vec2F(bordersAnchors.right, 0.0f), Vec2F(bordersAnchors.right, 1.0f), Vec2F(0, 0), Vec2F(1, 0));
		mBorderTopHandleWidget->layout = UIWidgetLayout(Vec2F(0.0f, bordersAnchors.top), Vec2F(1.0f, bordersAnchors.top), Vec2F(0, 0), Vec2F(0, 1));
		mBorderBottomHandleWidget->layout = UIWidgetLayout(Vec2F(0.0f, bordersAnchors.bottom), Vec2F(1.0f, bordersAnchors.bottom), Vec2F(0, 0), Vec2F(0, 1));
	}

	void ImageAssetPropertiesViewer::UpdateBordersValue()
	{
		BorderI value = mBordersSmoothValue;
		for (auto target : mTargetAssets)
			target->GetMeta()->mSliceBorder = value;

		mBorderProperty->Refresh();
		UpdateBordersAnchors();
	}

	void ImageAssetPropertiesViewer::SetupAtlasProperty()
	{
		if (mTargetAssets.IsEmpty())
			return;

		UID commonValue = mTargetAssets[0]->GetAtlasId();
		bool valuesDifferent = false;

		for (int i = 1; i < mTargetAssets.Count(); i++)
		{
			UID id = mTargetAssets[i]->GetAtlasId();
			if (id != commonValue)
			{
				valuesDifferent = true;
				break;
			}
		}

		if (valuesDifferent)
			mAtlasProperty->SetUnknownValue();
		else
			mAtlasProperty->SetAssetId(commonValue);
	}

	void ImageAssetPropertiesViewer::OnAtlasPropertyChanged()
	{
		UID id = mAtlasProperty->GetCommonValue().GetAssetId();

		for (auto target : mTargetAssets)
			target->SetAtlasId(id);
	}

}
 
CLASS_META(Editor::ImageAssetPropertiesViewer)
{
	BASE_CLASS(Editor::IAssetPropertiesViewer);

	PROTECTED_FIELD(mTargetAssets);
	PROTECTED_FIELD(mContent);
	PROTECTED_FIELD(mPreviewImageContent);
	PROTECTED_FIELD(mPreviewImage);
	PROTECTED_FIELD(mPreviewImageBack);
	PROTECTED_FIELD(mBorderLeftHandleWidget);
	PROTECTED_FIELD(mBorderRightHandleWidget);
	PROTECTED_FIELD(mBorderTopHandleWidget);
	PROTECTED_FIELD(mBorderBottomHandleWidget);
	PROTECTED_FIELD(mBorderLeftHandle);
	PROTECTED_FIELD(mBorderRightHandle);
	PROTECTED_FIELD(mBorderTopHandle);
	PROTECTED_FIELD(mBorderBottomHandle);
	PROTECTED_FIELD(mBordersSmoothValue);
	PROTECTED_FIELD(mBorderProperty);
	PROTECTED_FIELD(mDefaultTypeProperty);
	PROTECTED_FIELD(mAtlasProperty);
	PROTECTED_FIELD(mWindowsProperties);
	PROTECTED_FIELD(mOSXProperties);
	PROTECTED_FIELD(mAndroidProperties);
	PROTECTED_FIELD(mIOSProperties);

	PUBLIC_FUNCTION(void, SetTargetAssets, const Vector<Asset*>&);
	PUBLIC_FUNCTION(const Type*, GetAssetType);
	PUBLIC_FUNCTION(UIWidget*, GetWidget);
	PROTECTED_FUNCTION(void, InitializeImagePreview);
	PROTECTED_FUNCTION(void, InitializeProperties);
	PROTECTED_FUNCTION(void, FitImage);
	PROTECTED_FUNCTION(void, UpdateBordersAnchors);
	PROTECTED_FUNCTION(void, UpdateBordersValue);
	PROTECTED_FUNCTION(void, SetupAtlasProperty);
	PROTECTED_FUNCTION(void, OnAtlasPropertyChanged);
}
END_META;
 