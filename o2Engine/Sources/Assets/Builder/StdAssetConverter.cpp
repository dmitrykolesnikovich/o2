#include "StdAssetConverter.h"

#include "Assets/Assets.h"
#include "Assets/BinaryAsset.h"
#include "Assets/ImageAsset.h"
#include "Utils/FileSystem/FileSystem.h"

namespace o2
{
	Vector<const Type*> StdAssetConverter::GetProcessingAssetsTypes() const
	{
		Vector<const Type*> res;
		res.Add(&TypeOf(BinaryAsset));
		return res;
	}

	void StdAssetConverter::ConvertAsset(const AssetTree::AssetNode& node)
	{
		String sourceAssetPath = o2Assets.GetAssetsPath() + node.path;
		String buildedAssetPath = o2Assets.GetDataPath() + node.path;
		String sourceAssetMetaPath = sourceAssetPath + ".meta";
		String buildedAssetMetaPath = buildedAssetPath + ".meta";

		o2FileSystem.FileCopy(sourceAssetPath, buildedAssetPath);
		o2FileSystem.FileCopy(sourceAssetMetaPath, buildedAssetMetaPath);
		o2FileSystem.SetFileEditDate(buildedAssetPath, node.time);
	}

	void StdAssetConverter::RemoveAsset(const AssetTree::AssetNode& node)
	{
		String buildedAssetPath = o2Assets.GetDataPath() + node.path;
		String buildedAssetMetaPath = buildedAssetPath + ".meta";

		o2FileSystem.FileDelete(buildedAssetPath);
		o2FileSystem.FileDelete(buildedAssetMetaPath);
	}

	void StdAssetConverter::MoveAsset(const AssetTree::AssetNode& nodeFrom, const AssetTree::AssetNode& nodeTo)
	{
		String fullPathFrom = o2Assets.GetDataPath() + nodeFrom.path;
		String fullPathTo = o2Assets.GetDataPath() + nodeTo.path;
		String fullMetaPathFrom = fullPathFrom + ".meta";
		String fullMetaPathTo = fullPathTo + ".meta";

		o2FileSystem.FileMove(fullPathFrom, fullPathTo);
		o2FileSystem.FileMove(fullMetaPathFrom, fullMetaPathTo);
	}
}

CLASS_META(o2::StdAssetConverter)
{
	BASE_CLASS(o2::IAssetConverter);


	PUBLIC_FUNCTION(Vector<const Type*>, GetProcessingAssetsTypes);
	PUBLIC_FUNCTION(void, ConvertAsset, const AssetTree::AssetNode&);
	PUBLIC_FUNCTION(void, RemoveAsset, const AssetTree::AssetNode&);
	PUBLIC_FUNCTION(void, MoveAsset, const AssetTree::AssetNode&, const AssetTree::AssetNode&);
}
END_META;
