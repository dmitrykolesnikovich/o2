#include "DataAsset.h"

#include "Assets/Assets.h"

namespace o2
{

	DataAsset::DataAsset():
		Asset()
	{
		mMeta = mnew MetaInfo();
		InitializeProperties();
	}

	DataAsset::DataAsset(const String& path):
		Asset()
	{
		mPath = path;
		mMeta = mnew MetaInfo();
		IdRef() = o2Assets.GetAssetId(path);
		InitializeProperties();

		Load();
	}

	DataAsset::DataAsset(AssetId id):
		Asset()
	{
		mMeta = mnew MetaInfo();
		IdRef() = id;
		mPath = o2Assets.GetAssetPath(id);
		InitializeProperties();

		Load();
	}

	DataAsset::DataAsset(const DataAsset& asset):
		Asset(asset)
	{
		mMeta = mnew MetaInfo();
		mPath = asset.mPath;
		IdRef() = asset.GetAssetId();

		data = asset.data;

		InitializeProperties();
	}

	DataAsset::~DataAsset()
	{
	}

	DataAsset& DataAsset::operator=(const DataAsset& asset)
	{
		Asset::operator=(asset);

		data = asset.data;

		*mMeta = *(MetaInfo*)(asset.mMeta);

		return *this;
	}

	bool DataAsset::operator==(const DataAsset& other) const
	{
		return mMeta->IsEqual(other.mMeta);
	}

	bool DataAsset::operator!=(const DataAsset& other) const
	{
		return !mMeta->IsEqual(other.mMeta);
	}

	DataAsset::MetaInfo* DataAsset::GetMeta() const
	{
		return (MetaInfo*)mMeta;
	}

	const char* DataAsset::GetFileExtensions() const
	{
		return "xml cfg";
	}

	void DataAsset::LoadData(const String& path)
	{
		data.Clear();
		data.LoadFromFile(path);
	}

	void DataAsset::SaveData(const String& path)
	{
		data.SaveToFile(path);
	}

	void DataAsset::InitializeProperties()
	{
		INITIALIZE_GETTER(DataAsset, meta, GetMeta);
	}

	Type::Id DataAsset::MetaInfo::GetAssetType() const
	{
		return DataAsset::type.ID();
	}
}