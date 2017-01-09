#include "IntegerProperty.h"

#include "Application/Application.h"
#include "SceneWindow/SceneEditScreen.h"
#include "UI/EditBox.h"
#include "UI/UIManager.h"

namespace Editor
{
	IntegerProperty::IntegerProperty(UIWidget* widget /*= nullptr*/)
	{
		if (widget)
			mEditBox = dynamic_cast<UIEditBox*>(widget);
		else
			mEditBox = o2UI.CreateWidget<UIEditBox>("singleline with arrows");

		mEditBox->layout.minHeight = 10;
		mEditBox->onChangeCompleted = Function<void(const WString&)>(this, &IntegerProperty::OnEdited);
		mEditBox->text = "--";
		mEditBox->SetFilterInteger();

		auto handleLayer = mEditBox->GetLayer("arrows");

		if (handleLayer)
		{
			mEditBox->onDraw += [&]() { mDragHangle.OnDrawn(); };

			mDragHangle.cursorType = CursorType::SizeNS;
			mDragHangle.isUnderPoint = [=](const Vec2F& point) { return handleLayer->IsUnderPoint(point); };
			mDragHangle.onMoved = Function<void(const Input::Cursor&)>(this, &IntegerProperty::OnDragHandleMoved);
			mDragHangle.onCursorPressed = [&](const Input::Cursor&) { o2Application.SetCursorInfiniteMode(true); };
			mDragHangle.onCursorReleased = [&](const Input::Cursor&) { o2Application.SetCursorInfiniteMode(false); };
		}
	}

	IntegerProperty::~IntegerProperty()
	{
		delete mEditBox;
	}

	void IntegerProperty::SetValue(int value)
	{
		for (auto ptr : mValuesPointers)
			mAssignFunc(ptr, value);

		SetCommonValue(value);
	}

	void IntegerProperty::SetUnknownValue(int defaultValue /*= 0*/)
	{
		mValuesDifferent = true;
		mCommonValue = defaultValue;
		mEditBox->text = "--";

		onChanged();
		o2EditorSceneScreen.OnSceneChanged();
	}

	void IntegerProperty::Setup(const Vector<void*>& targets, bool isProperty)
	{
		if (isProperty)
		{
			mAssignFunc = [](void* ptr, const int& value) { *((Property<int>*)(ptr)) = value; };
			mGetFunc = [](void* ptr) { return ((Property<int>*)(ptr))->Get(); };
		}
		else
		{
			mAssignFunc = [](void* ptr, const int& value) { *((int*)(ptr)) = value; };
			mGetFunc = [](void* ptr) { return *((int*)(ptr)); };
		}

		mValuesPointers = targets;

		Refresh();
	}

	void IntegerProperty::Refresh()
	{
		if (mValuesPointers.IsEmpty())
			return;

		auto lastCommonValue = mCommonValue;
		auto lastDifferent = mValuesDifferent;

		auto newCommonValue = mGetFunc(mValuesPointers[0]);
		auto newDifferent = false;

		for (int i = 1; i < mValuesPointers.Count(); i++)
		{
			if (newCommonValue != mGetFunc(mValuesPointers[i]))
			{
				newDifferent = true;
				break;
			}
		}

		if (newDifferent)
		{
			if (!lastDifferent)
				SetUnknownValue(newCommonValue);
		}
		else if (newCommonValue != lastCommonValue || lastDifferent)
			SetCommonValue(newCommonValue);
	}

	UIWidget* IntegerProperty::GetWidget() const
	{
		return mEditBox;
	}

	int IntegerProperty::GetCommonValue() const
	{
		return mCommonValue;
	}

	bool IntegerProperty::IsValuesDifferent() const
	{
		return mValuesDifferent;
	}

	const Type* IntegerProperty::GetFieldType() const
	{
		return &TypeOf(int);
	}

	void IntegerProperty::SetCommonValue(int value)
	{
		mValuesDifferent = false;
		mCommonValue = value;
		mEditBox->text = (WString)mCommonValue;

		onChanged();
		o2EditorSceneScreen.OnSceneChanged();
	}

	void IntegerProperty::OnEdited(const WString& data)
	{
		if (mValuesDifferent && data == "--")
			return;

		SetValue((const int)data);
	}

	void IntegerProperty::OnDragHandleMoved(const Input::Cursor& cursor)
	{
		SetValue(mCommonValue + (int)cursor.delta.y);
	}

}
 
CLASS_META(Editor::IntegerProperty)
{
	BASE_CLASS(Editor::IPropertyField);

	PROTECTED_FIELD(mAssignFunc);
	PROTECTED_FIELD(mGetFunc);
	PROTECTED_FIELD(mValuesPointers);
	PROTECTED_FIELD(mCommonValue);
	PROTECTED_FIELD(mValuesDifferent);
	PROTECTED_FIELD(mEditBox);
	PROTECTED_FIELD(mDragHangle);

	PUBLIC_FUNCTION(void, Setup, const Vector<void*>&, bool);
	PUBLIC_FUNCTION(void, Refresh);
	PUBLIC_FUNCTION(UIWidget*, GetWidget);
	PUBLIC_FUNCTION(void, SetValue, int);
	PUBLIC_FUNCTION(void, SetUnknownValue, int);
	PUBLIC_FUNCTION(int, GetCommonValue);
	PUBLIC_FUNCTION(bool, IsValuesDifferent);
	PUBLIC_FUNCTION(const Type*, GetFieldType);
	PROTECTED_FUNCTION(void, SetCommonValue, int);
	PROTECTED_FUNCTION(void, OnEdited, const WString&);
	PROTECTED_FUNCTION(void, OnDragHandleMoved, const Input::Cursor&);
}
END_META;
 