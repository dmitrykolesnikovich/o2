#include "RectDrawable.h"

namespace o2
{
	IRectDrawable::IRectDrawable(const Vec2F& size /*= Vec2F()*/, const Vec2F& position /*= Vec2F()*/, 
								 float angle /*= 0.0f*/, const Vec2F& scale /*= Vec2F(1.0f, 1.0f)*/, 
								 const Color4& color /*= Color4::White()*/, const Vec2F& pivot /*= Vec2F()*/):
		Transform(size, position, angle, scale, pivot), mColor(color), mEnabled(true)
	{
		InitializeProperties();
	}

	IRectDrawable::IRectDrawable(const IRectDrawable& other):
		Transform(other.mSize, other.mPosition, other.mAngle, other.mScale, other.mPivot), 
		mColor(other.mColor), mEnabled(other.mEnabled)
	{
		InitializeProperties();
	}

	IRectDrawable& IRectDrawable::operator=(const IRectDrawable& other)
	{
		Transform::operator=(other);

		mColor = other.mColor;
		mEnabled = other.mEnabled;

		ColorChanged();
		EnableChanged();

		return *this;
	}

	void IRectDrawable::SetColor(const Color4& color)
	{
		mColor = color;
		ColorChanged();
	}

	Color4 IRectDrawable::GetColor() const
	{
		return mColor;
	}

	void IRectDrawable::SetTransparency(float transparency)
	{
		mColor.SetAF(transparency);
		ColorChanged();
	}

	float IRectDrawable::GetTransparency() const
	{
		return mColor.AF();
	}

	void IRectDrawable::SetEnabled(bool enabled)
	{
		mEnabled = enabled;
		EnableChanged();
	}

	bool IRectDrawable::IsEnabled() const
	{
		return mEnabled;
	}

	bool IRectDrawable::IsUnderPoint(const Vec2F& point)
	{
		return mDrawingScissorRect.IsInside(point) && Transform::IsPointInside(point);
	}

	void IRectDrawable::InitializeProperties()
	{
		INITIALIZE_PROPERTY(IRectDrawable, color, SetColor, GetColor);
		INITIALIZE_PROPERTY(IRectDrawable, transparency, SetTransparency, GetTransparency);
		INITIALIZE_PROPERTY(IRectDrawable, enabled, SetEnabled, IsEnabled);
	}
}

CLASS_META(o2::IRectDrawable)
{
	BASE_CLASS(o2::Transform);
	BASE_CLASS(o2::IDrawable);

	PUBLIC_FIELD(color);
	PUBLIC_FIELD(transparency);
	PUBLIC_FIELD(enabled);
	PROTECTED_FIELD(mColor).SERIALIZABLE_ATTRIBUTE();
	PROTECTED_FIELD(mEnabled).SERIALIZABLE_ATTRIBUTE();

	PUBLIC_FUNCTION(void, Draw);
	PUBLIC_FUNCTION(void, SetColor, const Color4&);
	PUBLIC_FUNCTION(Color4, GetColor);
	PUBLIC_FUNCTION(void, SetTransparency, float);
	PUBLIC_FUNCTION(float, GetTransparency);
	PUBLIC_FUNCTION(void, SetEnabled, bool);
	PUBLIC_FUNCTION(bool, IsEnabled);
	PUBLIC_FUNCTION(bool, IsUnderPoint, const Vec2F&);
	PROTECTED_FUNCTION(void, ColorChanged);
	PROTECTED_FUNCTION(void, EnableChanged);
	PROTECTED_FUNCTION(void, InitializeProperties);
}
END_META;
