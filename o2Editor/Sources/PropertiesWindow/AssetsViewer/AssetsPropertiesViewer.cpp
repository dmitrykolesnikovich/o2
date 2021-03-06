#include "AssetsPropertiesViewer.h"

#include "Assets/Assets.h"
#include "AssetsWindow/AssetsWindow.h"
#include "PropertiesWindow/AssetsViewer/DefaultAssetPropertiesViewer.h"
#include "PropertiesWindow/AssetsViewer/IAssetPropertiesViewer.h"
#include "Render/Sprite.h"
#include "UI/Button.h"
#include "UI/EditBox.h"
#include "UI/Image.h"
#include "UI/Label.h"
#include "UI/UIManager.h"
#include "UI/VerticalLayout.h"
#include "UI/Widget.h"
#include "Utils/FileSystem/FileSystem.h"

namespace Editor
{

	AssetsPropertiesViewer::AssetsPropertiesViewer()
	{
		auto viewersTypes = TypeOf(IAssetPropertiesViewer).GetDerivedTypes();
		for (auto type : viewersTypes)
		{
			if (*type != TypeOf(DefaultAssetPropertiesViewer))
				mAvailableAssetViewers.Add((IAssetPropertiesViewer*)(type->CreateSample()));
		}

		InitializeHeadWidget();
	}

	AssetsPropertiesViewer::~AssetsPropertiesViewer()
	{
		for (auto viewer : mAvailableAssetViewers)
			delete viewer;
	}

	const Type* AssetsPropertiesViewer::GetViewingObjectType() const
	{
		return &TypeOf(AssetRef);
	}

	void AssetsPropertiesViewer::AddAssetViewer(IAssetPropertiesViewer* viewer)
	{
		mAvailableAssetViewers.Add(viewer);
	}

	void AssetsPropertiesViewer::SetTargets(const Vector<IObject*> targets)
	{
		mTargetAssets = targets.Cast<AssetRef*>();

		if (mTargetAssets.IsEmpty())
			return;

		bool typesAreSame = true;
		const Type* commonType = &(*mTargetAssets[0])->GetType();
		for (int i = 1; i < mTargetAssets.Count(); i++)
		{
			const Type* assetType = &(*mTargetAssets[i])->GetType();

			if (assetType != commonType)
			{
				typesAreSame = false;
				break;
			}
		}

		if (!typesAreSame)
		{
			if (mCurrentViewer)
				mContentWidget->RemoveChild(mCurrentViewer->GetWidget(), false);

			mCurrentViewer = nullptr;
			return;
		}

		IAssetPropertiesViewer* viewer = mAvailableAssetViewers.FindMatch([&](IAssetPropertiesViewer* x) {
			return commonType->IsBasedOn(*x->GetAssetType());
		});

		if (!viewer)
		{
			auto defaultViewer = mnew DefaultAssetPropertiesViewer();
			defaultViewer->SpecializeAssetType(&(*mTargetAssets[0])->GetMeta()->GetType());
			mAvailableAssetViewers.Add(defaultViewer);
			viewer = defaultViewer;
		}

		if (viewer != mCurrentViewer)
		{
			if (mCurrentViewer)
				mContentWidget->RemoveChild(mCurrentViewer->GetWidget(), false, false);

			mCurrentViewer = viewer;

			if (mCurrentViewer)
			{
				mContentWidget->AddChild(mCurrentViewer->GetWidget());
				mCurrentViewer->SetTargetAssets(mTargetAssets);
			}
		}
		else if (mCurrentViewer)
			mCurrentViewer->SetTargetAssets(mTargetAssets);
	}

	void AssetsPropertiesViewer::OnEnabled()
	{}

	void AssetsPropertiesViewer::OnDisabled()
	{
		mTargetAssets.Clear();
	}

	void AssetsPropertiesViewer::Update(float dt)
	{}

	void AssetsPropertiesViewer::Draw()
	{}

	void AssetsPropertiesViewer::InitializeHeadWidget()
	{
		auto contentWidget = o2UI.CreateVerLayout();
		contentWidget->name          = "asset viewer layout";
		contentWidget->spacing       = 0.0f;
		contentWidget->border        = RectF();
		contentWidget->expandHeight  = false;
		contentWidget->expandWidth   = true;
		contentWidget->fitByChildren = true;
		contentWidget->baseCorner    = BaseCorner::Top;
		contentWidget->layout        = UIWidgetLayout::BothStretch();
		mContentWidget = contentWidget;
	}

	void AssetsPropertiesViewer::OnNameTextEditChanged(const WString& value)
	{
		if (mTargetAssets.Count() != 1)
			return;

		String ext = FileSystem::GetFileExtension((*mTargetAssets.Last())->GetPath());

		if (ext.IsEmpty())
			o2Assets.RenameAsset(*mTargetAssets.Last(), value);
		else
			o2Assets.RenameAsset(*mTargetAssets.Last(), value + "." + ext);
	}
}

CLASS_META(Editor::AssetsPropertiesViewer)
{
	BASE_CLASS(Editor::IObjectPropertiesViewer);

	PROTECTED_FIELD(mTargetAssets);
	PROTECTED_FIELD(mCurrentViewer);
	PROTECTED_FIELD(mAvailableAssetViewers);

	PUBLIC_FUNCTION(const Type*, GetViewingObjectType);
	PUBLIC_FUNCTION(void, AddAssetViewer, IAssetPropertiesViewer*);
	PROTECTED_FUNCTION(void, SetTargets, const Vector<IObject*>);
	PROTECTED_FUNCTION(void, OnEnabled);
	PROTECTED_FUNCTION(void, OnDisabled);
	PROTECTED_FUNCTION(void, Update, float);
	PROTECTED_FUNCTION(void, Draw);
	PROTECTED_FUNCTION(void, InitializeHeadWidget);
	PROTECTED_FUNCTION(void, OnNameTextEditChanged, const WString&);
}
END_META;
