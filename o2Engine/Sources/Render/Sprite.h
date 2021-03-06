#pragma once

#include "Assets/ImageAsset.h"
#include "Render/Mesh.h"
#include "Render/RectDrawable.h"
#include "Render/TextureRef.h"
#include "Utils/Math/Border.h"

namespace o2
{
	class Bitmap;

	// -----------------
	// Quad image sprite
	// -----------------
	class Sprite: public IRectDrawable
	{
	public:
		Property<TextureRef>    texture;          // Texture property
		Property<RectI>         textureSrcRect;   // Texture source rectangle property
		Property<ImageAssetRef> image;            // Sets image asset
		Property<String>        imageName;        // Sets image asset path
		Setter<Bitmap*>         bitmap;           // Sets image from bitmap
		Property<Color4>        leftTopColor;	  // Color of left top corner property
		Property<Color4>        rightTopColor;    // Color of right top corner property
		Property<Color4>        leftBottomColor;  // Color of left bottom corner property
		Property<Color4>        rightBottomColor; // Color of right bottom corner property
		Property<SpriteMode>    mode;             // Sprite drawing mode property
		Property<float>         fill;             // Sprite fill property
		Property<float>         tileScale;        // Sprite tile scale property, 1.0f is default
		Property<BorderI>       sliceBorder;      // Slice border property

		// Default constructor
		Sprite();

		// Constructor from image asset
		Sprite(const ImageAssetRef& image);

		// Constructor from image asset by path
		Sprite(const String& imagePath);

		// Constructor from image asset by id
		Sprite(UID imageId);

		// Constructor from texture and source rectangle
		Sprite(TextureRef texture, const RectI& srcRect);

		// Constructor from color
		Sprite(const Color4& color);

		// Constructor from bitmap
		Sprite(Bitmap* bitmap);

		// Copy-constructor
		Sprite(const Sprite& other);

		// Destructor
		~Sprite();

		// Assign operator
		Sprite& operator=(const Sprite& other);

		// Draws sprite 
		void Draw();

		// Sets using texture
		void SetTexture(TextureRef texture);

		// Returns using texture
		TextureRef GetTexture() const;

		// Sets texture source rectangle
		void SetTextureSrcRect(const RectI& rect);

		// Returns texture source rectangle
		RectI GetTextureSrcRect() const;

		// Returns original or texture source size
		Vec2I GetOriginalSize() const;

		// Sets corner color
		void SetCornerColor(Corner corner, const Color4& color);

		// Returns corner color
		Color4 GetCornerColor(Corner corner) const;

		// Sets left top corner color
		void SetLeftTopColor(const Color4& color);

		// Returns left top corner color
		Color4 GetLeftTopCorner() const;

		// Sets right top corner color
		void SetRightTopColor(const Color4& color);

		// Returns right top corner color
		Color4 GetRightTopCorner() const;

		// Sets right bottom corner color
		void SetRightBottomColor(const Color4& color);

		// Returns right bottom corner color
		Color4 GetRightBottomCorner() const;

		// Sets left bottom corner color
		void SetLeftBottomColor(const Color4& color);

		// Returns left bottom corner color
		Color4 GetLeftBottomCorner() const;

		// Sets sprite fill value (0 ... 1)
		void SetFill(float fill);

		// Returns sprite fill
		float GetFill() const;

		// Sets tile scale. 1.0f is default
		void SetTileScale(float scale);

		// Returns tile scale
		float GetTileScale() const;

		// Sets sprite drawing mode
		void SetMode(SpriteMode mode);

		// Returns sprite drawing mode
		SpriteMode GetMode() const;

		// Sets sprite slice border
		void SetSliceBorder(const BorderI& border);

		// Returns sprite slice border
		BorderI GetSliceBorder() const;

		// Loads sprite from image asset
		void LoadFromImage(const ImageAssetRef& image);

		// Loads sprite from image asset by path
		void LoadFromImage(const String& imagePath);

		// Loads sprite from image asset by id
		void LoadFromImage(UID imageId);

		// Loads sprite from mono color
		void LoadMonoColor(const Color4& color);

		// Loads sprite from bitmap
		void LoadFromBitmap(Bitmap* bitmap);

		// Sets asset
		void SetImageAsset(const ImageAssetRef& asset);

		// Returns asset
		ImageAssetRef GetImageAsset() const;

		// Returns image asset name
		String GetImageName() const;

		// Returns atlas asset id (returns 0 when sprite is not from atlas)
		UID GetAtlasAssetId() const;

		// Sets size by texture source rectangle size
		void NormalizeSize();

		// Sets size with equal aspect as texture source rectangle by width
		void NormalizeAspectByWidth();

		// Sets size with equal aspect as texture source rectangle by width
		void NormalizeAspectByHeight();

		// Sets size with equal aspect as texture source rectangle by nearest value
		void NormalizeAspect();

		// Calling when serializing
		void OnSerialize(DataNode& node) const;

		// Calling when deserializing
		void OnDeserialized(const DataNode& node);

		SERIALIZABLE(Sprite);

	protected:
		RectI         mTextureSrcRect;             // Texture source rectangle
		Color4        mCornersColors[4];           // Corners colors
		ImageAssetRef mImageAsset;                 // Image asset @SERIALIZABLE
		SpriteMode    mMode = SpriteMode::Default; // Drawing mode @SERIALIZABLE
		BorderI       mSlices;                     // Slice borders @SERIALIZABLE
		float         mFill = 1;                   // Sprite fillness @SERIALIZABLE
		float         mTileScale = 1;              // Scale of tiles in tiled mode. 1.0f is default and equals to default image size @SERIALIZABLE
		Mesh*         mMesh;                       // Drawing mesh

		void(Sprite::*mMeshBuildFunc)(); // Mesh building function pointer (by mode)

	protected:
		// It is called when basis was changed
		void BasisChanged();

		// It is called when color was changed
		void ColorChanged();

		// Updates mesh geometry
		void UpdateMesh();

		// Builds mesh for default mode
		void BuildDefaultMesh();

		// Builds mesh for sliced mode
		void BuildSlicedMesh();

		// Builds mesh for tiled mode
		void BuildTiledMesh();

		// Builds mesh for fill left to right mode
		void BuildFillLeftToRightMesh();

		// Builds mesh for fill right to left mode
		void BuildFillRightToLeftMesh();

		// Builds mesh for fill up to down mode
		void BuildFillUpToDownMesh();

		// Builds mesh for fill down to up mode
		void BuildFillDownToUpMesh();

		// Builds mesh for fill 360 clock wise mode
		void BuildFill360CWMesh();

		// Builds mesh for fill 360 counter clock wise mode
		void BuildFill360CCWMesh();

		// It is called when assets was rebuilded
		void ReloadImage();

		// Initializes properties
		void InitializeProperties();

		friend class Render;
	};
}
