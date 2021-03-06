#include "AnimationAsset.h"

#include "Assets/Assets.h"

namespace o2
{
	AnimationAsset::AnimationAsset():
		Asset()
	{
		mMeta = mnew MetaInfo();
		InitializeProperties();
	}

	AnimationAsset::AnimationAsset(const String& path):
		Asset()
	{
		mPath = path;
		mMeta = mnew MetaInfo();
		IdRef() = o2Assets.GetAssetId(path);
		InitializeProperties();

		Load();
	}

	AnimationAsset::AnimationAsset(UID id):
		Asset()
	{
		mMeta = mnew MetaInfo();
		IdRef() = id;
		mPath = o2Assets.GetAssetPath(id);
		InitializeProperties();

		Load();
	}

	AnimationAsset::AnimationAsset(const AnimationAsset& asset):
		Asset(asset)
	{
		animation = asset.animation;
		mMeta = mnew MetaInfo();
		mPath = asset.mPath;
		IdRef() = asset.GetAssetId();
		InitializeProperties();
	}

	AnimationAsset::~AnimationAsset()
	{}

	AnimationAsset& AnimationAsset::operator=(const AnimationAsset& asset)
	{
		Asset::operator=(asset);
		animation = asset.animation;

		*mMeta = *(MetaInfo*)(asset.mMeta);

		return *this;
	}

	bool AnimationAsset::operator==(const AnimationAsset& other) const
	{
		return mMeta->IsEqual(other.mMeta);
	}

	bool AnimationAsset::operator!=(const AnimationAsset& other) const
	{
		return !mMeta->IsEqual(other.mMeta);
	}

	AnimationAsset::MetaInfo* AnimationAsset::GetMeta() const
	{
		return (MetaInfo*)mMeta;
	}

	const char* AnimationAsset::GetFileExtensions() const
	{
		return "anim anm";
	}

	void AnimationAsset::LoadData(const String& path)
	{
		DataNode data;
		data.LoadFromFile(path);
		animation = data;
	}

	void AnimationAsset::SaveData(const String& path)
	{
		DataNode data;
		data = animation;
		data.SaveToFile(path);
	}

	void AnimationAsset::InitializeProperties()
	{
		INITIALIZE_GETTER(AnimationAsset, meta, GetMeta);
	}

	const Type* AnimationAsset::MetaInfo::GetAssetType() const
	{
		return &TypeOf(AnimationAsset);
	}

	AnimationAssetRef AnimationAssetRef::CreateAsset()
	{
		return o2Assets.CreateAsset<AnimationAsset>();
	}

}

CLASS_META(o2::AnimationAsset)
{
	BASE_CLASS(o2::Asset);

	PUBLIC_FIELD(animation);
	PUBLIC_FIELD(meta);

	PUBLIC_FUNCTION(MetaInfo*, GetMeta);
	PUBLIC_FUNCTION(const char*, GetFileExtensions);
	PROTECTED_FUNCTION(void, LoadData, const String&);
	PROTECTED_FUNCTION(void, SaveData, const String&);
	PROTECTED_FUNCTION(void, InitializeProperties);
}
END_META;

CLASS_META(o2::AnimationAssetRef)
{
	BASE_CLASS(o2::AssetRef);


	PUBLIC_FUNCTION(const Type&, GetAssetType);
}
END_META;

CLASS_META(o2::AnimationAsset::MetaInfo)
{
	BASE_CLASS(o2::Asset::IMetaInfo);


	PUBLIC_FUNCTION(const Type*, GetAssetType);
}
END_META;
