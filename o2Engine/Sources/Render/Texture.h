#pragma once

#include "Render/Windows/TextureBase.h"
#include "Utils/Math/Vector2.h"
#include "Utils/Property.h"
#include "Utils/String.h"
#include "Utils/UID.h"

namespace o2
{
	class Bitmap;
	class TextureRef;

	// -------
	// Texture
	// -------
	class Texture: public TextureBase
	{
	public:
		// Texture format
		enum class Format { Default, R8G8B8A8, R8G8B8 };

		// Texture usage
		enum class Usage { Default, RenderTarget };

	public:
		Getter<Vec2I>  size;     // Size of texture getter
		Getter<Format> format;   // texture format getter
		Getter<Usage>  usage;    // Texture usage getter
		Getter<String> fileName; // Texture file name getter

		// Default constructor
		Texture();

		// Constructor
		Texture(const Vec2I& size, Format format = Format::Default, Usage usage = Usage::Default);

		// Constructor from file
		Texture(const String& fileName);

		// Constructor from atlas page
		Texture(UID atlasAssetId, int page);

		// Constructor from atlas page
		Texture(const String& atlasAssetName, int page);

		// Constructor from bitmap
		Texture(Bitmap* bitmap);

		// Destructor
		~Texture();

		// Creates texture with size, format and usage
		void Create(const Vec2I& size, Format format = Format::Default, Usage usage = Usage::Default);

		// Creates texture from file
		void Create(const String& fileName);

		// Creates texture from atlas page
		void Create(UID atlasAssetId, int page);

		// Creates texture from atlas page
		void Create(const String& atlasAssetName, int page);

		// Creates texture from bitmap
		void Create(Bitmap* bitmap);

		// Sets texture's data from bitmap
		void SetData(Bitmap* bitmap);

		// Reloads texture
		void Reload();

		// Returns size of texture
		Vec2I GetSize() const;

		// Returns format
		Format GetFormat() const;

		// returns texture usage
		Usage GetUsage() const;

		// Returns texture file name
		String GetFileName() const;

		// Returns true when texture ready to use
		bool IsReady() const;

		// Returns is texture from atlas page
		bool IsAtlasPage() const;

		// Returns atlas asset id
		UID GetAtlasAssetId() const;

		// Returns atlas page
		int GetAtlasPage() const;

	protected:
		typedef Vector<TextureRef*> TextureRefsVec;

		Vec2I          mSize;         // Size of texture
		Format         mFormat;       // Texture format
		Usage          mUsage;        // Texture usage
		String         mFileName;     // Source file name
		UID        mAtlasAssetId; // Atlas asset id. Equals 0 if it isn't atlas texture
		int            mAtlasPage;    // Atlas page
		bool           mReady;        // Is texture ready to use
		TextureRefsVec mRefs;         // Texture references

	protected:
		// Initializes properties
		void InitializeProperties();

		friend class Render;
		friend class TextureRef;
	};
}
