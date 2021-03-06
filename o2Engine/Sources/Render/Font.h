#pragma once

#include "ft2build.h"
#include FT_FREETYPE_H

#include "Render/TextureRef.h"
#include "Utils/CommonTypes.h"
#include "Utils/Containers/Vector.h"
#include "Utils/Delegates.h"
#include "Utils/Math/Rect.h"
#include "Utils/Math/Vector2.h"
#include "Utils/String.h"

namespace o2
{
	class Mesh;
	class Render;
	class FontRef;

	// -----------------------------------------------------------
	// Font. Containing array of symbol glyphs, symbol index table
	// -----------------------------------------------------------
	class Font
	{
	protected:
		struct Character;

	public:
		Function<void()> onCharactersRebuild;

		// Default constructor
		Font();

		// Copy-constructor
		Font(const Font& font);

		// Destructor
		virtual ~Font();

		// Returns base height in pixels for font with size
		virtual float GetHeightPx(int height) const;

		// Returns line height in pixels for font with size
		virtual float GetLineHeightPx(int height) const;

		// Returns character constant reference by id
		virtual const Character& GetCharacter(UInt16 id, int height);

		// Checks characters for preloading
		virtual void CheckCharacters(const WString& needChararacters, int height);

		// Returns font file name
		virtual String GetFileName() const;

	protected:
		// --------------------
		// Character definition
		// --------------------
		struct Character
		{
			RectF  mTexSrc;  // texture source rect
			Vec2F  mSize;    // Size of source rect
			Vec2F  mOrigin;  // Symbol origin point
			float  mAdvance; // Symbol advance
			UInt16 mId;      // Character id
			int    mHeight;  // Character height

			bool operator==(const Character& other) const;
		};
		typedef Vector<Character> CharactersVec;

		typedef Vector<FontRef*> FontRefsVec;

	protected:
		FontRefsVec   mRefs;           // Array of reference to this font
		CharactersVec mCharacters;     // Characters array
		TextureRef    mTexture;        // Texture
		RectI         mTextureSrcRect; // Texture source rectangle
		bool          mReady;          // True when font is ready to use

		friend class Text;
		friend class FontRef;
		friend class Render;
	};
}
