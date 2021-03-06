#pragma once

#include "Assets/Asset.h"
#include "Assets/AssetsTree.h"
#include "Assets/Builder/StdAssetConverter.h"
#include "Utils/String.h"

namespace o2
{
	class FolderInfo;
	class IAssetConverter;

	// -------------
	// Asset builder
	// -------------
	class AssetsBuilder
	{
	public:
		typedef Vector<UID> AssetsIdsVec;

	public:
		// Default constructor
		AssetsBuilder();

		// Destructor
		~AssetsBuilder();

		// Builds asset from assets path to dataAssetsPath. Removes all builded assets if forcible is true
		AssetsIdsVec BuildAssets(const String& assetsPath, const String& dataAssetsPath, bool forcible = false);

	protected:
		typedef Dictionary<const Type*, IAssetConverter*> ConvertersDict;

		LogStream*           mLog;                 // Asset builder log stream

		String               mSourceAssetsPath;    // Source assets path
		AssetTree            mSourceAssetsTree;    // Source assets tree
		String               mBuildedAssetsPath;   // Builded assets path
		AssetTree            mBuildedAssetsTree;   // Builded assets tree

		AssetTree::AssetsVec mModifiedAssets;      // Modified assets infos

		ConvertersDict       mAssetConverters;     // Assets converters by type
		StdAssetConverter    mStdAssetConverter;   // Standard assets converter

	protected:
		// Initializes converters
		void InitializeConverters();

		// Removes all builded assets
		void RemoveBuildedAssets();

		// Checks basic atlas exist
		void CheckBasicAtlas();

		// Creates missing meta fields for source assets
		void CreateMissingMetas();

		// Searching and removing assets
		AssetsIdsVec ProcessRemovedAssets();

		// Searching modified and moved assets
		AssetsIdsVec ProcessModifiedAssets();

		// Searches new assets
		AssetsIdsVec ProcessNewAssets();

		// Launches converters post process
		AssetsIdsVec ConvertersPostProcess();
		
		// Processes folder for missing metas
		void ProcessMissingMetasCreation(FolderInfo& folder);
		
		// Generates meta information file for asset
		void GenerateMeta(const Type& assetType, const String& metaFullPath);

		// Returns assets converter by asset type
		IAssetConverter* GetAssetConverter(const Type* assetType);

		// Resets builder
		void Reset();

		friend class AtlasAssetConverter;
	};
}
