#include "FontRef.h"

#include "Assets/Assets.h"
#include "Assets/BitmapFontAsset.h"
#include "Assets/VectorFontAsset.h"
#include "Render/BitmapFont.h"
#include "Render/VectorFont.h"
#include "Utils/FileSystem/FileSystem.h"

namespace o2
{
	FontRef::FontRef():
		mFont(nullptr)
	{}

	FontRef::FontRef(Font* font):
		mFont(font)
	{
		if (mFont)
			mFont->mRefs.Add(this);
	}

	FontRef::FontRef(const String& fileName):
		mFont(nullptr)
	{
		String ext = o2FileSystem.GetFileExtension(fileName);
		if (ext == "xml")
		{
			BitmapFontAssetRef fontAsset(fileName);
			*this = fontAsset->GetFont();
		}
		else
		{
			VectorFontAssetRef fontAsset(fileName);
			*this = fontAsset->GetFont();
		}
	}

	FontRef::FontRef(const FontRef& other):
		mFont(other.mFont)
	{
		if (mFont)
			mFont->mRefs.Add(this);
	}

	FontRef::~FontRef()
	{
		if (mFont)
			mFont->mRefs.Remove(this);
	}

	Font* FontRef::operator->()
	{
		return mFont;
	}

	const Font* FontRef::operator->() const
	{
		return mFont;
	}

	Font& FontRef::operator*()
	{
		return *mFont;
	}

	Font* FontRef::Get()
	{
		return mFont;
	}

	const Font* FontRef::Get() const
	{
		return mFont;
	}

	bool FontRef::IsValid() const
	{
		return mFont != nullptr;
	}

	FontRef::operator bool() const
	{
		return mFont != nullptr;
	}

	bool FontRef::operator!=(const FontRef& other) const
	{
		return mFont != other.mFont;
	}

	bool FontRef::operator==(const FontRef& other) const
	{
		return mFont == other.mFont;
	}

	FontRef& FontRef::operator=(const FontRef& other)
	{
		if (mFont)
			mFont->mRefs.Remove(this);

		mFont = other.mFont;

		if (mFont)
			mFont->mRefs.Add(this);

		return *this;
	}
}