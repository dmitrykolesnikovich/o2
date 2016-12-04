#include "AtlasAssetConverter.h"

#include "Assets/Assets.h"
#include "Assets/AtlasAsset.h"
#include "Assets/ImageAsset.h"
#include "Utils/Bitmap.h"
#include "Utils/FileSystem/FileSystem.h"
#include "Utils/Log/LogStream.h"

namespace o2
{
	Vector<Type::Id> AtlasAssetConverter::GetProcessingAssetsTypes() const
	{
		Vector<Type::Id> res;
		res.Add(TypeOf(AtlasAsset).ID());
		return res;
	}

	void AtlasAssetConverter::ConvertAsset(const AssetTree::AssetNode& node)
	{
		String sourceAssetPath = o2Assets.GetAssetsPath() + node.mPath;
		String buildedAssetPath = o2Assets.GetDataPath() + node.mPath;
		String sourceAssetMetaPath = sourceAssetPath + ".meta";
		String buildedAssetMetaPath = buildedAssetPath + ".meta";

		if (!o2FileSystem.IsFileExist(buildedAssetPath))
			o2FileSystem.WriteFile(buildedAssetPath, "");

		o2FileSystem.FileCopy(sourceAssetMetaPath, buildedAssetMetaPath);
	}

	void AtlasAssetConverter::RemoveAsset(const AssetTree::AssetNode& node)
	{
		String buildedAssetPath = o2Assets.GetDataPath() + node.mPath;
		String buildedAssetMetaPath = buildedAssetPath + ".meta";

		DataNode atlasData;
		atlasData.LoadFromFile(buildedAssetPath);
		int pagesCount = atlasData["mPages"].GetChildNodes().Count();

		for (int i = 0; i < pagesCount; i++)
			o2FileSystem.FileDelete(buildedAssetPath + (String)i + ".png");

		o2FileSystem.FileDelete(buildedAssetPath);
		o2FileSystem.FileDelete(buildedAssetMetaPath);
	}

	void AtlasAssetConverter::MoveAsset(const AssetTree::AssetNode& nodeFrom, const AssetTree::AssetNode& nodeTo)
	{
		String fullPathFrom = o2Assets.GetDataPath() + nodeFrom.mPath;
		String fullPathTo = o2Assets.GetDataPath() + nodeTo.mPath;
		String fullMetaPathFrom = fullPathFrom + ".meta";
		String fullMetaPathTo = fullPathTo + ".meta";

		DataNode atlasData;
		atlasData.LoadFromFile(fullPathFrom);
		int pagesCount = atlasData["mPages"].GetChildNodes().Count();

		for (int i = 0; i < pagesCount; i++)
			o2FileSystem.FileMove(fullPathFrom + (String)i + ".png",
								  fullPathTo + (String)i + ".png");

		o2FileSystem.FileMove(fullPathFrom, fullPathTo);
		o2FileSystem.FileMove(fullMetaPathFrom, fullMetaPathTo);
	}

	Vector<UID> AtlasAssetConverter::AssetsPostProcess()
	{
		CheckBasicAtlas();
		return CheckRebuildingAtlases();
	}

	void AtlasAssetConverter::Reset()
	{}

	void AtlasAssetConverter::CheckBasicAtlas()
	{
		Type::Id imageTypeId = TypeOf(ImageAsset).ID();
		Type::Id atlasTypeId = TypeOf(AtlasAsset).ID();

		AssetTree::AssetNode* basicAtlasInfo = nullptr;

		Vector<UID> availableAtlasesIds;
		for (auto assetInfo : mAssetsBuilder->mBuildedAssetsTree.mAllAssets)
		{
			if (assetInfo->mType == atlasTypeId)
			{
				availableAtlasesIds.Add(assetInfo->mMeta->ID());
			}

			if (assetInfo->mPath == BASIC_ATLAS_PATH)
				basicAtlasInfo = assetInfo;
		}

		for (auto assetInfo : mAssetsBuilder->mBuildedAssetsTree.mAllAssets)
		{
			if (assetInfo->mType == imageTypeId)
			{
				ImageAsset::MetaInfo* imageMeta = (ImageAsset::MetaInfo*)assetInfo->mMeta;

				if (!availableAtlasesIds.Contains(imageMeta->mAtlasId))
					imageMeta->mAtlasId = basicAtlasInfo->mMeta->ID();
			}
		}
	}

	Vector<UID> AtlasAssetConverter::CheckRebuildingAtlases()
	{
		Vector<UID> res;
		Type::Id atlasAssetTypeId = TypeOf(AtlasAsset).ID();

		for (auto info : mAssetsBuilder->mBuildedAssetsTree.mAllAssets)
		{
			if (info->mType == atlasAssetTypeId)
				if (CheckAtlasRebuilding(info))
					res.Add(info->mId);
		}

		return res;
	}

	bool AtlasAssetConverter::CheckAtlasRebuilding(AssetTree::AssetNode* atlasInfo)
	{
		DataNode atlasData;
		atlasData.LoadFromFile(mAssetsBuilder->mBuildedAssetsPath + atlasInfo->mPath);

		ImagesVec lastImages;
		lastImages = atlasData["AllImages"];

		ImagesVec currentImages;
		Type::Id imageTypeId = TypeOf(ImageAsset).ID();
		UID atlasId = atlasInfo->mMeta->ID();
		for (auto assetInfo : mAssetsBuilder->mBuildedAssetsTree.mAllAssets)
		{
			if (assetInfo->mType == imageTypeId)
			{
				ImageAsset::MetaInfo* imageMeta = (ImageAsset::MetaInfo*)assetInfo->mMeta;
				if (imageMeta->mAtlasId == atlasId)
				{
					currentImages.Add(Image(imageMeta->ID(), assetInfo->mTime));
				}
			}
		}

		if (IsAtlasNeedRebuild(currentImages, lastImages))
		{
			RebuildAtlas(atlasInfo, currentImages);
			return true;
		}

		return false;
	}

	bool AtlasAssetConverter::IsAtlasNeedRebuild(ImagesVec& currentImages, ImagesVec& lastImages)
	{
		if (currentImages.Count() != lastImages.Count())
			return true;

		for (auto lastImg : lastImages)
		{
			bool found = false;
			for (auto curImg : currentImages)
			{
				if (lastImg.mId == curImg.mId)
				{
					if (lastImg.mTime != curImg.mTime)
					{
						return true;
					}
					else
					{
						found = true;
						break;
					}
				}
			}

			if (!found)
				return true;
		}

		for (auto curImg : currentImages)
		{
			if (mAssetsBuilder->mModifiedAssets.ContainsPred([&](auto x) { return x->mId == curImg.mId; }))
				return true;
		}

		return false;
	}

	void AtlasAssetConverter::RebuildAtlas(AssetTree::AssetNode* atlasInfo, ImagesVec& images)
	{
		auto meta = (AtlasAsset::MetaInfo*)atlasInfo->mMeta;

		RectsPacker packer(meta->mWindows.mMaxSize);
		float imagesBorder = (float)meta->mBorder;

		// Initialize pack images
		ImagePackDefsVec packImages;
		for (auto img : images)
		{
			// Find image info
			AssetTree::AssetNode* imgInfo =
				mAssetsBuilder->mBuildedAssetsTree.mAllAssets.FindMatch([&](auto info) { return info->mMeta->ID() == img.mId; });

			if (!imgInfo)
			{
				mAssetsBuilder->mLog->Error("Can't find asset info by id: %i", img.mId);
				continue;
			}

			String assetFullPath = mAssetsBuilder->mSourceAssetsPath + imgInfo->mPath;

			// Load bitmap
			Bitmap* bitmap = mnew Bitmap();
			if (!bitmap->Load(assetFullPath))
			{
				mAssetsBuilder->mLog->Error("Can't load bitmap for image asset: %s", imgInfo->mPath);
				delete bitmap;
				continue;
			}

			// Create packing rect
			RectsPacker::Rect* packRect = packer.AddRect(bitmap->GetSize() +
														 Vec2F(imagesBorder*2.0f, imagesBorder*2.0f));

			ImagePackDef imagePackDef;
			imagePackDef.mAssetInfo = imgInfo;
			imagePackDef.mBitmap = bitmap;
			imagePackDef.mPackRect = packRect;

			packImages.Add(imagePackDef);
		}

		// Try to pack
		if (!packer.Pack())
		{
			mAssetsBuilder->mLog->Error("Atlas %s packing failed", atlasInfo->mPath);
			return;
		}
		else
			mAssetsBuilder->mLog->Out("Atlas %s successfully packed", atlasInfo->mPath);

		// Initialize bitmaps and pages
		int pagesCount = packer.GetPagesCount();
		Vector<Bitmap*> resAtlasBitmaps;
		Vector<AtlasAsset::Page> resAtlasPages;
		for (int i = 0; i < pagesCount; i++)
		{
			AtlasAsset::Page atlasPage;
			atlasPage.mId = 0;
			atlasPage.mSize = packer.GetMaxSize();
			resAtlasPages.Add(atlasPage);

			Bitmap* newBitmap = mnew Bitmap(Bitmap::Format::R8G8B8A8, packer.GetMaxSize());
			newBitmap->Fill(Color4(255, 255, 255, 0));

			resAtlasBitmaps.Add(newBitmap);
		}

		// Save image assets data and fill pages
		AssetInfosVec atlasImagesInfos;
		for (auto imgDef : packImages)
		{
			imgDef.mPackRect->mRect.left += imagesBorder;
			imgDef.mPackRect->mRect.right -= imagesBorder;
			imgDef.mPackRect->mRect.top -= imagesBorder;
			imgDef.mPackRect->mRect.bottom += imagesBorder;

			atlasImagesInfos.Add(AssetInfo(imgDef.mAssetInfo->mPath, imgDef.mAssetInfo->mMeta->ID(), TypeOf(ImageAsset).ID()));

			resAtlasBitmaps[imgDef.mPackRect->mPage]->CopyImage(imgDef.mBitmap,
																imgDef.mPackRect->mRect.LeftBottom());

			resAtlasPages[imgDef.mPackRect->mPage].mImagesRects.Add(imgDef.mAssetInfo->mMeta->ID(),
																	imgDef.mPackRect->mRect);

			SaveImageAsset(imgDef);
		}

		// Save pages bitmaps
		for (int i = 0; i < pagesCount; i++)
		{
			resAtlasBitmaps[i]->Save(mAssetsBuilder->mBuildedAssetsPath + atlasInfo->mPath + (String)i + ".png",
									 Bitmap::ImageType::Png);

			delete resAtlasBitmaps[i];
		}

		// Save atlas data
		DataNode atlasData;
		atlasData["mPages"] = resAtlasPages;
		atlasData["AllImages"] = images;
		atlasData["mImagesAssetsInfos"] = atlasImagesInfos;

		String atlasFullPath = mAssetsBuilder->mBuildedAssetsPath + atlasInfo->mPath;
		atlasData.SaveToFile(atlasFullPath);
		o2FileSystem.SetFileEditDate(atlasFullPath, atlasInfo->mTime);
	}

	void AtlasAssetConverter::SaveImageAsset(ImagePackDef& imgDef)
	{
		DataNode imgData;
		imgData["mAtlasPage"] = 0;
		imgData["mAtlasRect"] = (RectI)(imgDef.mPackRect->mRect);
		String imageFullPath = mAssetsBuilder->mBuildedAssetsPath + imgDef.mAssetInfo->mPath;
		imgData.SaveToFile(imageFullPath);
		o2FileSystem.SetFileEditDate(imageFullPath, imgDef.mAssetInfo->mTime);

		DataNode metaData;
		metaData = imgDef.mAssetInfo->mMeta;
		metaData.SaveToFile(mAssetsBuilder->mBuildedAssetsPath + imgDef.mAssetInfo->mPath + ".meta");
		metaData.SaveToFile(mAssetsBuilder->mSourceAssetsPath + imgDef.mAssetInfo->mPath + ".meta");
	}

	AtlasAssetConverter::Image::Image(UID id, const TimeStamp& time):
		mId(id), mTime(time)
	{}

	bool AtlasAssetConverter::Image::operator==(const Image& other) const
	{
		return mId == other.mId;
	}

	bool AtlasAssetConverter::ImagePackDef::operator==(const ImagePackDef& other) const
	{
		return mAssetInfo == other.mAssetInfo && mBitmap == other.mBitmap && mPackRect == other.mPackRect;
	}
}
 
CLASS_META(o2::AtlasAssetConverter)
{
	BASE_CLASS(o2::IAssetConverter);


	PUBLIC_FUNCTION(Vector<Type::Id>, GetProcessingAssetsTypes);
	PUBLIC_FUNCTION(void, ConvertAsset, const AssetTree::AssetNode&);
	PUBLIC_FUNCTION(void, RemoveAsset, const AssetTree::AssetNode&);
	PUBLIC_FUNCTION(void, MoveAsset, const AssetTree::AssetNode&, const AssetTree::AssetNode&);
	PUBLIC_FUNCTION(Vector<UID>, AssetsPostProcess);
	PUBLIC_FUNCTION(void, Reset);
	PROTECTED_FUNCTION(void, CheckBasicAtlas);
	PROTECTED_FUNCTION(Vector<UID>, CheckRebuildingAtlases);
	PROTECTED_FUNCTION(bool, CheckAtlasRebuilding, AssetTree::AssetNode*);
	PROTECTED_FUNCTION(bool, IsAtlasNeedRebuild, ImagesVec&, ImagesVec&);
	PROTECTED_FUNCTION(void, RebuildAtlas, AssetTree::AssetNode*, ImagesVec&);
	PROTECTED_FUNCTION(void, SaveImageAsset, ImagePackDef&);
}
END_META;

CLASS_META(o2::AtlasAssetConverter::Image)
{
	BASE_CLASS(o2::ISerializable);

	PUBLIC_FIELD(mId).SERIALIZABLE_ATTRIBUTE();
	PUBLIC_FIELD(mTime).SERIALIZABLE_ATTRIBUTE();
}
END_META;
 