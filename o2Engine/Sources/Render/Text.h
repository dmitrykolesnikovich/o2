#pragma once

#include "Assets/Asset.h"
#include "Assets/BitmapFontAsset.h"
#include "Assets/VectorFontAsset.h"
#include "Render/FontRef.h"
#include "Render/RectDrawable.h"
#include "Utils/String.h"

namespace o2
{
	class Mesh;
	class Render;

	// ------------------------------------------------------------------------------------------
	// Text renderer class. Using font, basis and many style parameters. Caching text into meshes
	// ------------------------------------------------------------------------------------------
	class Text: public IRectDrawable
	{
	public:
		struct SymbolsSet;
		
	public:
		Property<FontRef>  font;                // Font pointer property
		Property<WString>  text;                // Text property, wstring
		Property<int>      height;              // Text height
		Property<VerAlign> verAlign;            // vertical align property
		Property<HorAlign> horAlign;            // Horizontal align property
		Property<bool>     wordWrap;            // Words wrapping flag property
		Property<bool>     dotsEngings;         // Dots engings when overflow property
		Property<float>    symbolsDistanceCoef; // Characters distance coef, 1 is standard
		Property<float>    linesDistanceCoef;   // Lines distance coef, 1 is standard

		// Default constructor
		Text();

		// Constructor
		Text(FontRef font);

		// Constructor
		Text(const String& fontFileName);

		// Constructor
		Text(UID fontAssetId);

		// Constructor
		Text(const BitmapFontAssetRef& fontAsset);

		// Constructor
		Text(const VectorFontAssetRef& fontAsset);

		// Copy-constructor
		Text(const Text& text);

		// Destructor
		~Text();

		// Assign operator
		Text& operator=(const Text& other);

		// Draw text
		void Draw();

		// Sets using font
		void SetFont(FontRef font);

		// Returns using font
		FontRef GetFont() const;

		// Sets bitmap font asset 
		void SetFontAsset(const BitmapFontAssetRef& asset);

		// Sets vector font asset
		void SetFontAsset(const VectorFontAssetRef& asset);

		// Sets font asset id (loads asset by this id)
		void SetFontAsset(UID assetId);

		// Sets font asset (loads asset by file name)
		void SetFontAsset(const String& fileName);

		// Returns asset by font asset id
		AssetRef GetFontAsset() const;

		// Returns font asset id
		UID GetFontAssetId() const;

		// Sets font height
		void SetHeight(int height);

		// Returns font height
		int GetHeight() const;

		// Sets text
		void SetText(const WString& text);

		// Returns text
		WString GetText() const;

		// Sets horizontal align
		void SetHorAlign(HorAlign align);

		// Returns horizontal align
		HorAlign GetHorAlign() const;

		// Sets vertical align
		void SetVerAlign(VerAlign align);

		// returns vertical align
		VerAlign GetVerAlign() const;

		// Sets word wrapping
		void SetWordWrap(bool flag);

		// Returns word wrapping
		bool GetWordWrap() const;

		// Sets dots endings
		void SetDotsEngings(bool flag);

		// Returns dots endings
		bool IsDotsEngings() const;

		// Sets characters distance coefficient
		void SetSymbolsDistanceCoef(float coef = 1);

		// Returns characters distance coef
		float GetSymbolsDistanceCoef() const;

		// Sets lines distance coefficient
		void SetLinesDistanceCoef(float coef = 1);

		// Returns lines distance coefficient
		float GetLinesDistanceCoef() const;

		// Returns symbol set structure pointer
		SymbolsSet& GetSymbolsSet();

		// Returns real text size
		Vec2F GetRealSize();

		// Returns real text rectangle
		RectF GetRealRect();

		// Returns text size
		static Vec2F GetTextSize(const WString& text, Font* font, int height = 11,
								 const Vec2F& areaSize = Vec2F(), 
								 HorAlign horAlign = HorAlign::Left, VerAlign verAlign = VerAlign::Top, 
								 bool wordWrap = true, bool dotsEngings = false, float charsDistCoef = 1.0f, 
								 float linesDistCoef = 1.0f);

		SERIALIZABLE(Text);

	public:
		// ----------------
		// Text symbols set
		// ----------------
		struct SymbolsSet
		{
			// ----------------------------------
			// Simple symbol definition structure
			// ----------------------------------
			struct Symbol
			{
				RectF  mFrame;   // Frame of symbol layout
				RectF  mTexSrc;  // Texture source rect
				UInt16 mCharId;  // Character id
				Vec2F  mOrigin;  // Character offset
				float  mAdvance; // Character advance

			public:
				// Default constructor
				Symbol();

				// Constructor
				Symbol(const Vec2F& position, const Vec2F& size, const RectF& texSrc, UInt16 charId,
						  const Vec2F& origin, float advance);

				// Equals operator
				bool operator==(const Symbol& other) const;
			};
			typedef Vector<Symbol> SymbolDefsVec;

			// -------------------------
			// Line definition structure
			// -------------------------
			struct Line
			{
				SymbolDefsVec mSymbols;       // Symbols in line
				WString       mString;        // Line string
				Vec2F         mSize;          // Size of line in pixels
				Vec2F         mPosition;      // Position of line
				int           mLineBegSymbol; // Index of line beginning symbol
				int           mSpacesCount;   // Spaces count at line
				bool          mEndedNewLine;  // True, if line ended by new line character

			public:
				// Default constructor
				Line();

				// Equals operator
				bool operator==(const Line& other) const;
			};
			typedef Vector<Line> LineDefsVec;

		public:
			FontRef     mFont;            // Font
			int         mHeight;          // Text height
			WString     mText;            // Text string
			Vec2F       mPosition;        // Position, in pixels
			Vec2F       mAreaSize;        // Area size, in pixels
			Vec2F       mRealSize;        // Real text size
			HorAlign    mHorAlign;        // Horizontal align
			VerAlign    mVerAlign;        // Vertical align
			bool        mWordWrap;        // True, when words wrapping
			bool        mDotsEndings;     // Dots ending when overflow
			float       mSymbolsDistCoef; // Characters distance coefficient, 1 is standard
			float       mLinesDistCoef;   // Lines distance coefficient, 1 is standard
			LineDefsVec mLines;           // Lines definitions

		public:
			// Calculating characters layout by parameters
			void Initialize(FontRef font, const WString& text, int height, const Vec2F& position, const Vec2F& areaSize, 
							HorAlign horAlign, VerAlign verAlign, bool wordWrap, bool dotsEngings, float charsDistCoef, 
							float linesDistCoef);

			// Moves symbols 
			void Move(const Vec2F& offs);
		};

	protected:
		typedef Vector<Mesh*> MeshesVec;

		static char* mBasicSymbolsPreset;
		const UInt mMeshMaxPolyCount = 4096;

		WString    mText;              // Wide char string, containing rendering text @SERIALIZABLE
		UID        mFontAssetId;       // Font asset id @SERIALIZABLE
		FontRef    mFont;              // Using font
		int        mHeight;            // Text height @SERIALIZABLE
		float      mSymbolsDistCoef;   // Characters distance coef, 1 is standard @SERIALIZABLE
		float      mLinesDistanceCoef; // Lines distance coef, 1 is standard @SERIALIZABLE
		VerAlign   mVerAlign;          // Vertical align @SERIALIZABLE
		HorAlign   mHorAlign;          // Horizontal align @SERIALIZABLE
		bool       mWordWrap;          // True, when words wrapping @SERIALIZABLE
		bool       mDotsEndings;       // If true, text will end on '...' @SERIALIZABLE
									   
		MeshesVec  mMeshes;            // Meshes vector
		Basis      mLastTransform;     // Last mesh update transformation
									   
		SymbolsSet mSymbolsSet;        // Symbols set definition
									   
		bool       mUpdatingMesh;      // True, when mesh is already updating

	protected:
		// Updating meshes
		void UpdateMesh();

		// Transforming meshes by basis
		void TransformMesh(const Basis& bas);

		// Preparing meshes for characters count
		void PrepareMesh(int charactersCount);

		// Calculates and returns text basis
		Basis CalculateTextBasis() const;

		// It is called when color was changed
		void ColorChanged();

		// It is called when basis was changed
		void BasisChanged();

		// Calling when deserializing
		void OnDeserialized(const DataNode& node);

		// Initializing properties
		void InitializeProperties();
	};
}
