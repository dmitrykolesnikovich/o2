#pragma once

#include "Assets/AssetInfo.h"
#include "Utils/Containers/Vector.h"
#include "Utils/Memory/Ptr.h"
#include "Utils/Property.h"
#include "Utils/Serialization.h"
#include "Utils/Singleton.h"

namespace o2
{

	// Assets system access macros
#define  o2Assets Assets::Instance()

	class Asset;
	class LogStream;
	class AssetsBuilder;

	// ----------------
	// Assets utilities
	// ----------------
	class Assets: public Singleton<Assets>
	{
		friend class Asset;

	public:
		typedef Dictionary<String, Type*> TypesExtsDict;

	public:
		Getter<String> AssetsPath; // Assets path getter

		// Default constructor
		Assets();

		// Destructor
		~Assets();

		// Returns assets path
		String GetAssetsPath() const;

		// Returns data path
		String GetDataPath() const;

		// Returns asset path by asset id
		String GetAssetPath(UInt id) const;

		// Returns asset id by path
		UInt GetAssetId(const String& path) const;

		// Returns asset info by id
		AssetInfo GetAssetInfo(UInt id) const;

		// Returns asset info by path
		AssetInfo GetAssetInfo(const String& path) const;

		// Returns assets types and extensions dictionary. Key - extension, value - type of asset
		const TypesExtsDict GetAssetsExtensionsTypes() const;

		// Returns standard asset type
		const Type* GetStdAssetType() const;

		// Returns asset type for extension
		const Type* GetAssetTypeByExtension(const String& extension) const;

		// Creates new asset
		template<typename _asset_type>
		Ptr<_asset_type> CreateAsset();

		// Loads new asset by path
		template<typename _asset_type>
		Ptr<_asset_type> LoadAsset(const String& path);

		// Loads new asset by id
		template<typename _asset_type>
		Ptr<_asset_type> LoadAsset(UInt id);

		// Loads asset by info
		Ptr<Asset> LoadAsset(const AssetInfo& info);

		// Returns true if asset exist by path
		bool IsAssetExist(const String& path) const;

		// Returns true if asset exist by id
		bool IsAssetExist(UInt id) const;

		// Returns true if asset exist
		bool IsAssetExist(const AssetInfo& info) const;

		// Removes asset
		bool RemoveAsset(Ptr<Asset> asset, bool rebuildAssets = true);

		// Removes asset by path
		bool RemoveAsset(const String& path, bool rebuildAssets = true);

		// Removes asset by id
		bool RemoveAsset(UInt id, bool rebuildAssets = true);

		// Removes asset by info
		bool RemoveAsset(const AssetInfo& info, bool rebuildAssets = true);

		// Moves asset to new path
		bool MoveAsset(Ptr<Asset> asset, const String& newPath, bool rebuildAssets = true);

		// Moves asset by path to new path
		bool MoveAsset(const String& path, const String& newPath, bool rebuildAssets = true);

		// Moves asset by id to new path
		bool MoveAsset(UInt id, const String& newPath, bool rebuildAssets = true);

		// Moves asset to new path
		bool MoveAsset(const AssetInfo& info, const String& newPath, bool rebuildAssets = true);

		// Rebuilds all assets
		void RebuildAssets(bool forcible = false);

		// Returns random asset id
		static UInt GetRandomAssetId();

	protected:
		AssetInfosVec      mAssetsInfos;   // Assets infos
		Ptr<LogStream>     mLog;           // Log stream
		Ptr<AssetsBuilder> mAssetsBuilder; // Assets builder
		TypesExtsDict      mAssetsTypes;   // Assets types and extensions dictionary
		Type*              mStdAssetType;  // Standard asset type

	protected:
		// Loads asset infos
		void LoadAssetsInfos();

		// Initializes types extensions dictionary
		void LoadAssetTypes();

		// Initializes properties
		void InitializeProperties();
	};


	template<typename _asset_type>
	Ptr<_asset_type> Assets::CreateAsset()
	{
		return mnew _asset_type();
	}

	template<typename _asset_type>
	Ptr<_asset_type> Assets::LoadAsset(const String& path)
	{
		return mnew _asset_type(path);
	}

	template<typename _asset_type>
	Ptr<_asset_type> Assets::LoadAsset(UInt id)
	{
		return mnew _asset_type(id);
	}
}