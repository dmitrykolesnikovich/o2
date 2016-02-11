#include "EditBox.h"

#include "Application/Application.h"
#include "Render/Mesh.h"
#include "Render/Render.h"
#include "Render/Sprite.h"
#include "Render/Text.h"
#include "UI/HorizontalScrollBar.h"
#include "UI/UIManager.h"
#include "UI/VerticalScrollBar.h"
#include "Utils/Clipboard.h"
#include "Utils/Debug.h"
#include "Utils/Time.h"

namespace o2
{
	UIEditBox::UIEditBox():
		UIScrollArea(), mSelectionBegin(0), mSelectionEnd(0), mMultiLine(true), mWordWrap(false), mMaxLineChars(0),
		mMaxLinesCount(0), mSelectionColor(0.1f, 0.2f, 0.6f, 0.3f), mCaretBlinkDelay(1), mCaretBlinkTime(0),
		mLastClickTime(-10.0f), DrawableCursorEventsListener(this)
	{
		mSelectionMesh = mnew Mesh();
		mTextDrawable = mnew Text();
		mCaretDrawable = mnew Sprite();
		InitializeProperties();
	}

	UIEditBox::UIEditBox(const UIEditBox& other):
		UIScrollArea(other), mMultiLine(other.mMultiLine), mWordWrap(other.mWordWrap), mMaxLineChars(other.mMaxLineChars),
		mMaxLinesCount(other.mMaxLinesCount), mSelectionBegin(0), mSelectionEnd(0), mText(other.mText), mLastText(other.mText),
		mAvailableSymbols(other.mAvailableSymbols), mSelectionColor(other.mSelectionColor),
		mCaretBlinkDelay(other.mCaretBlinkDelay), mCaretBlinkTime(0), mLastClickTime(-10.0f), 
		DrawableCursorEventsListener(this)
	{
		mSelectionMesh = mnew Mesh();
		mTextDrawable = other.mTextDrawable->Clone();
		mCaretDrawable = other.mCaretDrawable->Clone();

		mTextDrawable->SetText(mText);

		RetargetStatesAnimations();
		UpdateLayout();

		InitializeProperties();
	}

	UIEditBox::~UIEditBox()
	{
		delete mSelectionMesh;
		delete mTextDrawable;
		delete mCaretDrawable;
	}

	UIEditBox& UIEditBox::operator=(const UIEditBox& other)
	{
		UIScrollArea::operator=(other);

		delete mTextDrawable;
		delete mCaretDrawable;

		mText = other.mText;
		mLastText = mText;
		mAvailableSymbols = other.mAvailableSymbols;
		mSelectionBegin = 0;
		mSelectionEnd = 0;
		mCaretBlinkTime = 0;
		mLastClickTime = -10.0f;
		mMultiLine = other.mMultiLine;
		mWordWrap = other.mWordWrap;
		mMaxLineChars = other.mMaxLineChars;
		mMaxLinesCount = other.mMaxLinesCount;
		mSelectionColor = other.mSelectionColor;
		mCaretBlinkDelay = other.mCaretBlinkDelay;
		mTextDrawable = other.mTextDrawable->Clone();
		mCaretDrawable = other.mCaretDrawable->Clone();

		mTextDrawable->SetText(mText);

		RetargetStatesAnimations();
		UpdateLayout();

		onChanged(mText);

		return *this;
	}

	void UIEditBox::Draw()
	{
		if (mFullyDisabled)
			return;

		for (auto layer : mDrawingLayers)
			layer->Draw();

		IDrawable::OnDrawn();

		mDrawDepth = o2Render.GetDrawingDepth();

		o2Render.EnableScissorTest(mAbsoluteClipArea);

		mTextDrawable->Draw();
		mSelectionMesh->Draw();

		if (mIsSelected)
			mCaretDrawable->Draw();

		for (auto child : mChilds)
			child->Draw();

		o2Render.DisableScissorTest();

		for (auto layer : mTopDrawingLayers)
			layer->Draw();

		if (mOwnHorScrollBar)
			mHorScrollBar->Draw();

		if (mOwnVerScrollBar)
			mVerScrollBar->Draw();

		if (UI_DEBUG || o2Input.IsKeyDown(VK_F1))
			DrawDebugFrame();
	}

	void UIEditBox::Update(float dt)
	{
		if (mFullyDisabled)
			return;

		UIScrollArea::Update(dt);
		UpdateCaretBlinking(dt);

		if (mIsSelected && o2Input.IsCursorReleased() && !UIWidget::IsUnderPoint(o2Input.GetCursorPos()))
			UIWidget::Deselect();
	}

	void UIEditBox::SetText(const WString& text)
	{
		mText = GetFilteredText(text);
		mTextDrawable->SetText(mText);
		UpdateScrollParams();
		UpdateSelectionAndCaret();
		onChanged(mText);

		mLastText = mText;
	}

	WString UIEditBox::GetText() const
	{
		return mText;
	}

	void UIEditBox::SetCaretPosition(int caretPosition)
	{
		mSelectionBegin = mSelectionEnd = caretPosition;
		UpdateSelectionAndCaret();
	}

	int UIEditBox::GetCaretPosition()
	{
		return mSelectionEnd;
	}

	void UIEditBox::Select(int begin, int end)
	{
		mSelectionBegin = begin;
		mSelectionEnd = end;
		UpdateSelectionAndCaret();
	}

	void UIEditBox::SetSelectionBegin(int position)
	{
		mSelectionBegin = position;
		UpdateSelectionAndCaret();
	}

	int UIEditBox::GetSelectionBegin() const
	{
		return mSelectionBegin;
	}

	void UIEditBox::SetSelectionEnd(int position)
	{
		mSelectionEnd = position;
		UpdateSelectionAndCaret();
	}

	int UIEditBox::GetSelectionEnd() const
	{
		return mSelectionEnd;
	}

	void UIEditBox::Deselect()
	{
		mSelectionBegin = mSelectionEnd;
		UpdateSelectionAndCaret();
	}

	void UIEditBox::SelectAll()
	{
		mSelectionBegin = 0;
		mSelectionEnd = mText.Length();

		UpdateSelectionAndCaret();
		CheckScrollingToCaret();
	}

	Text* UIEditBox::GetTextDrawable()
	{
		return mTextDrawable;
	}

	Sprite* UIEditBox::GetCaretDrawable()
	{
		return mCaretDrawable;
	}

	void UIEditBox::SetSelectionColor(const Color4& color)
	{
		mSelectionColor = color;
		UpdateSelectionAndCaret();
	}

	Color4 UIEditBox::GetSelectionColor() const
	{
		return mSelectionColor;
	}

	void UIEditBox::SetFilterInteger()
	{
		SetAvailableSymbols("1234567890");
	}

	void UIEditBox::SetFilterFloat()
	{
		SetAvailableSymbols("1234567890.,");
	}

	void UIEditBox::SetFilterNames()
	{
		SetAvailableSymbols("1234567890qwertyuioplkjhgfdsazxcvbnm_ QWERTYUIOPLKJHGFDSAZXCVBNM.,");
	}

	void UIEditBox::SetAvailableSymbols(const WString& availableSymbols)
	{
		mAvailableSymbols = availableSymbols;

		if (mAvailableSymbols.Length() > 0)
		{
			mText = GetFilteredText(mText);
			mTextDrawable->SetText(mText);

			UpdateLayout();
		}

		onChanged(mText);
	}

	WString UIEditBox::GetAvailableSymbols() const
	{
		return mAvailableSymbols;
	}

	void UIEditBox::SetMaxSizes(int maxLineCharactersCount, int maxLinesCount)
	{
		mMaxLineChars = maxLineCharactersCount;
		mMaxLinesCount = maxLinesCount;

		CheckCharactersAndLinesBounds();
	}

	void UIEditBox::CheckCharactersAndLinesBounds()
	{
		int lines = 0;
		int lineChars = 0;
		WString filteredText(mText.Capacity() + 1);
		int fi = 0;
		for (int i = 0; i < mText.Length(); i++)
		{
			if (mText[i] == '\n')
			{
				lines++;
				lineChars = 0;
				if (lines == mMaxLinesCount && mMaxLinesCount > 0)
					break;
			}

			lineChars++;

			if (lineChars < mMaxLineChars || mMaxLineChars == 0 || mText[i] == '\n')
				filteredText[fi++] = mText[i];
		}

		filteredText[fi] = '\0';

		mText = filteredText;
		mTextDrawable->SetText(filteredText);
		onChanged(mText);

		UpdateLayout();
	}

	void UIEditBox::SetMaxLineCharactersCount(int count)
	{
		mMaxLineChars = count;
		CheckCharactersAndLinesBounds();
	}

	int UIEditBox::GetMaxLineCharactersCount() const
	{
		return mMaxLineChars;
	}

	void UIEditBox::SetMaxLinesCount(int count)
	{
		mMaxLinesCount = count;
		CheckCharactersAndLinesBounds();
	}

	int UIEditBox::GetMaxLinesCount() const
	{
		return mMaxLinesCount;
	}

	void UIEditBox::SetMultiLine(bool multiline)
	{
		mMultiLine = multiline;

		if (!mMultiLine)
		{
			WString filteredText(mText.Capacity() + 1);
			int fi = 0;
			for (int i = 0; i < mText.Length(); i++)
			{
				if (mText[i] != '\n')
					filteredText[fi++] = mText[i];
			}

			filteredText[fi] = '\0';

			mText = filteredText;
			mLastText = mText;
			mTextDrawable->SetText(filteredText);

			UpdateLayout();
		}

		onChanged(mText);
	}

	bool UIEditBox::IsMultiLine() const
	{
		return mMultiLine;
	}

	void UIEditBox::SetWordWrap(bool wordWrap)
	{
		mWordWrap = wordWrap;
		mTextDrawable->SetWordWrap(mWordWrap);
	}

	bool UIEditBox::IsWordWrap() const
	{
		return mWordWrap;
	}

	void UIEditBox::SetCaretBlinkingDelay(float delay)
	{
		mCaretBlinkDelay = delay;
	}

	float UIEditBox::GetCaretBlinkingDelay() const
	{
		return mCaretBlinkDelay;
	}

	bool UIEditBox::IsScrollable() const
	{
		return mEnableHorScroll || mEnableVerScroll;
	}

	bool UIEditBox::IsSelectable() const
	{
		return true;
	}

	void UIEditBox::UpdateControls(float dt)
	{}

	void UIEditBox::OnVisibleChanged()
	{
		interactable = mResVisible;
	}

	void UIEditBox::OnDeselected()
	{
		mLastText = mText;
		onChangeCompleted(mText);
	}

	void UIEditBox::OnCursorPressed(const Input::Cursor& cursor)
	{
		o2UI.SelectWidget(this);

		auto pressedState = state["pressed"];
		if (pressedState)
			*pressedState = true;

		if (o2Time.GetApplicationTime() - mLastClickTime < 0.3f || o2Input.IsKeyDown(VK_CONTROL))
		{
			mSelectionBegin = mSelectionEnd = GetTextCaretPosition(cursor.mPosition);
			JumpSelection(false, false);
			JumpSelection(true, true);

			mSelWordBegin = Math::Min(mSelectionBegin, mSelectionEnd);
			mSelWordEnd = Math::Max(mSelectionBegin, mSelectionEnd);

			mSelectingByWords = true;
		}
		else
		{
			mSelectionBegin = mSelectionEnd = GetTextCaretPosition(cursor.mPosition);
			mSelectingByWords = false;

			UpdateSelectionAndCaret();
			CheckScrollingToCaret();
		}

		mLastClickTime = o2Time.GetApplicationTime();
		mCaretBlinkTime = 0.0f;
		mLastCursorPos = cursor.mPosition;
	}

	void UIEditBox::OnCursorReleased(const Input::Cursor& cursor)
	{
		auto pressedState = state["pressed"];
		if (pressedState)
			*pressedState = false;

		if (!UIWidget::IsUnderPoint(cursor.mPosition))
		{
			o2Application.SetCursor(CursorType::Arrow);
			UIWidget::Deselect();
		}
	}

	void UIEditBox::OnCursorPressBreak(const Input::Cursor& cursor)
	{
		auto pressedState = state["pressed"];
		if (pressedState)
			*pressedState = false;

		if (!UIWidget::IsUnderPoint(cursor.mPosition))
			o2Application.SetCursor(CursorType::Arrow);
	}

	void UIEditBox::OnCursorStillDown(const Input::Cursor& cursor)
	{
		if ((cursor.mPosition - mLastCursorPos).Length() < 3.0f)
			return;

		if (mSelectingByWords)
		{
			int curIdx = GetTextCaretPosition(cursor.mPosition);

			if (curIdx > mSelWordEnd)
			{
				mSelectionBegin = mSelWordBegin;
				mSelectionEnd = curIdx;
				JumpSelection(true, true);
			}
			else if (curIdx < mSelWordBegin)
			{
				mSelectionBegin = mSelWordEnd;
				mSelectionEnd = curIdx;
				JumpSelection(false, true);
			}
			else
			{
				mSelectionBegin = mSelWordBegin;
				mSelectionEnd = mSelWordEnd;

				UpdateSelectionAndCaret();
				CheckScrollingToCaret();
			}
		}
		else
		{
			mSelectionEnd = GetTextCaretPosition(cursor.mPosition);
			UpdateSelectionAndCaret();
			CheckScrollingToCaret();
		}

		mCaretBlinkTime = 0;
		mLastCursorPos = cursor.mPosition;
	}

	void UIEditBox::OnCursorEnter(const Input::Cursor& cursor)
	{
		auto selectState = state["select"];
		if (selectState)
			*selectState = true;

		if (!cursor.mPressed)
			o2Application.SetCursor(CursorType::IBeam);
	}

	void UIEditBox::OnCursorExit(const Input::Cursor& cursor)
	{
		auto selectState = state["select"];
		if (selectState)
			*selectState = false;

		if (!IsPressed() && !cursor.mPressed)
			o2Application.SetCursor(CursorType::Arrow);
	}

	void UIEditBox::OnCursorRightMousePressed(const Input::Cursor& cursor)
	{

	}

	void UIEditBox::OnCursorRightMouseStayDown(const Input::Cursor& cursor)
	{

	}

	void UIEditBox::OnCursorRightMouseReleased(const Input::Cursor& cursor)
	{

	}

	void UIEditBox::OnScrolled(float scroll)
	{
		if (mVerScrollBar && mEnableVerScroll)
			mVerScrollBar->OnScrolled(scroll);
		else if (mHorScrollBar && mEnableVerScroll)
			mHorScrollBar->OnScrolled(scroll);
	}

	void UIEditBox::OnKeyPressed(const Input::Key& key)
	{
		if (!mIsSelected)
			return;

		CheckCharacterTyping(key.mKey);
		CheckCaretMoving(key.mKey);
		CheckCharactersErasing(key.mKey);
		CheckClipboard(key.mKey);
	}

	void UIEditBox::OnKeyReleased(const Input::Key& key)
	{
		if (mIsSelected)
		{
			if (key.mKey == VK_ESCAPE)
			{
				if (!mMultiLine)
					SetText(mLastText);

				UIWidget::Deselect();
			}

			if (!mMultiLine)
			{
				if (key.mKey == VK_RETURN)
				{
					mLastText = mText;
					UIWidget::Deselect();
				}
			}
		}
	}

	void UIEditBox::OnKeyStayDown(const Input::Key& key)
	{
		if (!mIsSelected)
			return;

		if (!o2Input.IsKeyRepeating(key.mKey))
			return;

		CheckCharacterTyping(key.mKey);
		CheckCaretMoving(key.mKey);
		CheckCharactersErasing(key.mKey);
		CheckClipboard(key.mKey);
	}

	WString UIEditBox::GetFilteredText(const WString& text)
	{
		if (mAvailableSymbols.Length() == 0)
			return text;

		WString filteredText(text.Capacity() + 1);
		for (int i = 0; i < text.Length(); i++)
		{
			bool pass = false;
			for (int j = 0; j < mAvailableSymbols.Length(); j++)
			{
				if (mAvailableSymbols[j] == text[i])
				{
					pass = true;
					break;
				}
			}

			if (pass)
				filteredText += text[i];
		}

		filteredText += '\0';

		return filteredText;
	}

	void UIEditBox::UpdateScrollParams()
	{
		if (mTextDrawable->GetFont())
			mTextDrawable->GetFont()->CheckCharacters(" ");

		mAbsoluteViewArea = mViewAreaLayout.Calculate(layout.mAbsoluteRect);
		RectF localViewArea(0.0f, 0.0f, mAbsoluteViewArea.Width(), mAbsoluteViewArea.Height());

		mScrollArea = RectF(0.0f, 0.0f, localViewArea.Width(), localViewArea.Height());

		for (auto child : mChilds)
		{
			mScrollArea.left = Math::Min(mScrollArea.left, child->layout.mLocalRect.left);
			mScrollArea.bottom = Math::Min(mScrollArea.bottom, child->layout.mLocalRect.bottom);
			mScrollArea.right = Math::Max(mScrollArea.right, child->layout.mLocalRect.right);
			mScrollArea.top = Math::Max(mScrollArea.top, child->layout.mLocalRect.top);
		}

		RectF localTextLayout = localViewArea;
		Vec2F textRealSize = mTextDrawable->GetRealSize();
		RectF textArea(localTextLayout.left, localTextLayout.top - textRealSize.y,
					   localTextLayout.left + textRealSize.x, localTextLayout.top);

		mScrollArea.left = Math::Min(mScrollArea.left, textArea.left);
		mScrollArea.bottom = Math::Min(mScrollArea.bottom, textArea.bottom);
		mScrollArea.right = Math::Max(mScrollArea.right, textArea.right);
		mScrollArea.top = Math::Max(mScrollArea.top, textArea.top);

		mScrollRange = RectF(mScrollArea.left - localViewArea.left,
							 localViewArea.Height() - mScrollArea.top + localViewArea.bottom,
							 -(localViewArea.Width() - mScrollArea.right + localViewArea.left),
							 -mScrollArea.bottom + localViewArea.bottom);

		if (mHorScrollBar)
		{
			if (Math::Equals(mScrollRange.left, mScrollRange.right, 1.0f))
			{
				if (mEnableHorScroll)
				{
					auto enableHorBarState = state["enableHorBar"];
					if (enableHorBarState)
						*enableHorBarState = false;

					mHorScrollBar->Hide();
				}

				mEnableHorScroll = false;
			}
			else
			{
				if (!mEnableHorScroll)
				{
					auto enableHorBarState = state["enableHorBar"];
					if (enableHorBarState)
						*enableHorBarState = true;

					mHorScrollBar->Show();
				}

				mEnableHorScroll = true;

				mHorScrollBar->SetValueRange(mScrollRange.left, mScrollRange.right);
				mHorScrollBar->SetScrollHandleSize(localViewArea.Width());
			}
		}

		if (mVerScrollBar)
		{
			if (Math::Equals(mScrollRange.bottom, mScrollRange.top, 1.0f))
			{
				if (mEnableVerScroll)
				{
					auto enableVerBarState = state["enableVerBar"];
					if (enableVerBarState)
						*enableVerBarState = false;

					mVerScrollBar->Hide();
				}

				mEnableVerScroll = false;
			}
			else
			{
				if (!mEnableVerScroll)
				{
					auto enableVerBarState = state["enableVerBar"];
					if (enableVerBarState)
						*enableVerBarState = true;

					mVerScrollBar->Show();
				}

				mEnableVerScroll = true;

				mVerScrollBar->SetValueRange(mScrollRange.bottom, mScrollRange.top);
				mVerScrollBar->SetScrollHandleSize(localViewArea.Height());
			}
		}
	}

	void UIEditBox::UpdateLayout(bool forcible /*= false*/)
	{
		if (layout.mDrivenByParent && !forcible)
		{
			if (mParent)
				mParent->UpdateLayout();

			return;
		}

		RecalculateAbsRect();
		UpdateLayersLayouts();

		mAbsoluteViewArea = mViewAreaLayout.Calculate(layout.mAbsoluteRect);
		mAbsoluteClipArea = mClipAreaLayout.Calculate(layout.mAbsoluteRect);
		Vec2F roundedScrollPos(-Math::Round(mScrollPos.x), Math::Round(mScrollPos.y));

		mTextDrawable->SetRect(mAbsoluteViewArea + roundedScrollPos);

		mChildsAbsRect = mAbsoluteViewArea + roundedScrollPos;

		for (auto child : mChilds)
			child->UpdateLayout();

		UpdateScrollParams();

		RectF _mChildsAbsRect = mChildsAbsRect;
		mChildsAbsRect = layout.mAbsoluteRect;

		if (mOwnHorScrollBar)
			mHorScrollBar->UpdateLayout(true);

		if (mOwnVerScrollBar)
			mVerScrollBar->UpdateLayout(true);

		mChildsAbsRect = _mChildsAbsRect;

		UpdateSelectionAndCaret();
	}

	void UIEditBox::UpdateTransparency()
	{
		UIWidget::UpdateTransparency();
		mTextDrawable->transparency = mResTransparency;

		Color4 sc = mSelectionColor;
		sc.a = (int)(((float)sc.a)*mResTransparency);
		ULong selectionClr = sc.ABGR();
		for (UInt i = 0; i < mSelectionMesh->vertexCount; i++)
			mSelectionMesh->vertices[i].color = selectionClr;
	}

	void UIEditBox::UpdateSelectionAndCaret()
	{
		Vec2F caretPosition = GetTextCaretPosition(mSelectionEnd);
		mCaretDrawable->SetPosition(caretPosition);

		mSelectionMesh->vertexCount = 0;
		mSelectionMesh->polyCount = 0;

		if (mSelectionBegin == mSelectionEnd)
			return;

		int beg = Math::Min(mSelectionBegin, mSelectionEnd);
		int end = Math::Max(mSelectionBegin, mSelectionEnd);
		float caretDown = -mCaretDrawable->GetSize().y*mCaretDrawable->GetPivot().y;
		float caretUp = caretDown + mCaretDrawable->GetSize().y;

		auto& symbolsSet = mTextDrawable->GetSymbolsSet();
		auto font = mTextDrawable->GetFont();
		float spaceAdvance = font->GetCharacter(' ').mAdvance;

		for (auto line : symbolsSet.mLines)
		{
			if (beg > line.mLineBegSymbol + line.mSymbols.Count() || end < line.mLineBegSymbol)
				continue;

			if (line.mSymbols.Count() == 0)
			{
				Vec2F lb = line.mPosition + Vec2F(0, caretDown);
				Vec2F rt = lb + Vec2F(spaceAdvance, caretUp);
				AddSelectionRect(RectF(lb, rt));
				continue;
			}

			int begSymbol = Math::Max(beg - line.mLineBegSymbol, 0);
			int endSymbol = end - line.mLineBegSymbol;
			float endOffs = 0, begOffs = 0;
			int lineSymbolsCount = line.mSymbols.Count();

			if (begSymbol >= lineSymbolsCount)
			{
				begOffs = line.mSymbols.Last().mAdvance;
				begSymbol = line.mSymbols.Count() - 1;
			}

			if (endSymbol >= lineSymbolsCount)
			{
				endOffs = line.mSymbols.Last().mAdvance;
				if (line.mEndedNewLine && endSymbol > lineSymbolsCount)
					endOffs += spaceAdvance;

				endSymbol = lineSymbolsCount - 1;
			}

			Vec2F lb(line.mSymbols[begSymbol].mFrame.left + line.mSymbols[begSymbol].mOrigin.x + begOffs,
					 line.mPosition.y + caretDown);
			Vec2F rt(line.mSymbols[endSymbol].mFrame.left + line.mSymbols[endSymbol].mOrigin.x + endOffs,
					 line.mPosition.y + caretUp);

			AddSelectionRect(RectF(lb, rt));
		}
	}

	Vec2F UIEditBox::GetTextCaretPosition(int position)
	{
		bool fakeSymbols = false;
		if (mText.Length() == 0)
		{
			fakeSymbols = true;
			mTextDrawable->SetText("A");
		}

		auto& symbolsSet = mTextDrawable->GetSymbolsSet();
		for (auto line : symbolsSet.mLines)
		{
			if (position >= line.mLineBegSymbol && position <= line.mLineBegSymbol + line.mSymbols.Count())
			{
				int off = position - line.mLineBegSymbol;

				if (off < line.mSymbols.Count())
				{
					auto symb = line.mSymbols[off];
					auto res = symb.mFrame.LeftBottom() + symb.mOrigin;
					if (fakeSymbols)
						mTextDrawable->SetText("");
					return res;
				}
				else
				{
					if (line.mSymbols.Count() == 0)
						return line.mPosition;

					auto symb = line.mSymbols.Last();
					auto res = symb.mFrame.LeftBottom() + symb.mOrigin + Vec2F(symb.mAdvance, 0);
					if (fakeSymbols)
						mTextDrawable->SetText("");
					return res;
				}
			}
		}

		if (auto fnt = mTextDrawable->GetFont())
		{
			if (fakeSymbols)
				mTextDrawable->SetText("");

			return mAbsoluteViewArea.LeftTop() - Vec2F(0, mTextDrawable->GetFont()->GetHeight());
		}

		return Vec2F();
	}

	int UIEditBox::GetTextCaretPosition(const Vec2F& point)
	{
		auto& symbolsSet = mTextDrawable->GetSymbolsSet();
		auto font = mTextDrawable->GetFont();
		float lineHeight = font->GetLineHeight();
		float lineOffCoef = 0.25f;

		bool checkUp, checkDown, checkLeft, checkRight;
		int lineIdx = 0;
		for (auto line : symbolsSet.mLines)
		{
			checkUp = lineIdx > 0;
			checkDown = lineIdx < (int)symbolsSet.mLines.Count() - 1;

			float lineTop = (line.mSize.y*(1.0f - lineOffCoef) + lineHeight)*0.5f + line.mPosition.y;
			float lineBottom = -line.mSize.y*lineOffCoef + line.mPosition.y;

			if (line.mSymbols.Count() == 0 && point.y > lineBottom && point.y < lineTop)
				return line.mLineBegSymbol;


			int idx = 0;
			for (auto symb : line.mSymbols)
			{
				checkLeft = idx > 0;
				checkRight = idx < (int)line.mSymbols.Count() - 1;

				RectF sf(symb.mFrame.left, lineTop, symb.mFrame.right, lineBottom);

				bool ls = checkLeft ? sf.left < point.x : true;
				bool rs = checkRight ? sf.right > point.x : true;
				bool ts = checkUp ? sf.top > point.y:true;
				bool bs = checkDown ? sf.bottom < point.y : true;

				if (ls && rs && ts && bs)
				{
					if (point.x > (symb.mFrame.left + symb.mFrame.right)*0.5f)
						return line.mLineBegSymbol + idx + 1;
					else
						return line.mLineBegSymbol + idx;
				}

				idx++;
			}

			lineIdx++;
		}

		return 0;
	}

	void UIEditBox::UpdateCaretBlinking(float dt)
	{
		mCaretBlinkTime += dt;

		float blinkAlpha = Math::Clamp01(1.0f - (mCaretBlinkTime - mCaretBlinkDelay*0.3f) / (mCaretBlinkDelay*0.3f));
		mCaretDrawable->SetTransparency(blinkAlpha*mResTransparency);

		if (mCaretBlinkTime > mCaretBlinkDelay)
			mCaretBlinkTime -= mCaretBlinkDelay;
	}

	void UIEditBox::AddSelectionRect(const RectF& rect)
	{
		if (mSelectionMesh->GetMaxPolyCount() < mSelectionMesh->polyCount + 6)
		{
			int newPolyCount = mSelectionMesh->polyCount + 6;
			mSelectionMesh->Resize(newPolyCount * 2, newPolyCount);
		}

		unsigned long color = mSelectionColor.ABGR();

		mSelectionMesh->vertices[mSelectionMesh->vertexCount++] = Vertex2(rect.LeftBottom(), color, 0.0f, 0.0f);
		mSelectionMesh->vertices[mSelectionMesh->vertexCount++] = Vertex2(rect.LeftTop(), color, 0.0f, 0.0f);
		mSelectionMesh->vertices[mSelectionMesh->vertexCount++] = Vertex2(rect.RightTop(), color, 0.0f, 0.0f);
		mSelectionMesh->vertices[mSelectionMesh->vertexCount++] = Vertex2(rect.RightBottom(), color, 0.0f, 0.0f);

		mSelectionMesh->indexes[mSelectionMesh->polyCount * 3] = mSelectionMesh->vertexCount - 4;
		mSelectionMesh->indexes[mSelectionMesh->polyCount * 3 + 1] = mSelectionMesh->vertexCount - 3;
		mSelectionMesh->indexes[mSelectionMesh->polyCount * 3 + 2] = mSelectionMesh->vertexCount - 2;
		mSelectionMesh->polyCount++;

		mSelectionMesh->indexes[mSelectionMesh->polyCount * 3] = mSelectionMesh->vertexCount - 4;
		mSelectionMesh->indexes[mSelectionMesh->polyCount * 3 + 1] = mSelectionMesh->vertexCount - 2;
		mSelectionMesh->indexes[mSelectionMesh->polyCount * 3 + 2] = mSelectionMesh->vertexCount - 1;
		mSelectionMesh->polyCount++;
	}

	void UIEditBox::CheckScrollingToCaret()
	{
		auto font = mTextDrawable->GetFont();
		if (!font)
			return;

		Vec2F caretPos = mCaretDrawable->GetPosition();
		RectF clipRect = mAbsoluteViewArea;

		float rightOffs = Math::Max(caretPos.x - clipRect.right + 5.0f, 0.0f);
		float leftOffs = Math::Max(clipRect.left - caretPos.x + 5.0f, 0.0f);

		float downOffs = Math::Max(caretPos.y - clipRect.top + font->GetHeight(), 0.0f);
		float topOffs = Math::Max(clipRect.bottom - caretPos.y, 0.0f);

		float horOffs = rightOffs - leftOffs;

		if (!Math::Equals(horOffs, 0.0f))
		{
			if (mHorScrollBar)
				mHorScrollBar->SetValue(mScrollPos.x + horOffs);
			else
			{
				mScrollPos.x = Math::Clamp(mScrollPos.x + horOffs, mScrollRange.left, mScrollRange.right);
				UpdateLayout();
			}
		}

		if (mMultiLine)
		{
			float verOffs = topOffs - downOffs;

			if (!Math::Equals(verOffs, 0.0f))
			{
				if (mVerScrollBar)
					mVerScrollBar->SetValue(mScrollPos.y + verOffs);
				else
				{
					mScrollPos.y = Math::Clamp(mScrollPos.y + verOffs, mScrollRange.bottom, mScrollRange.top);
					UpdateLayout();
				}
			}
		}
	}

	void UIEditBox::JumpSelection(bool forward, bool selecting)
	{
		static char jumpSymbols[] = " \n()-=_+\\|/**&^%$#@!~,.?";
		int jumpSymbolsCount = strlen(jumpSymbols);

		int jumpIdx = mSelectionEnd + 1;

		if (forward)
		{
			bool stop = false;
			for (int i = mSelectionEnd; i < mText.Length() && !stop; i++)
			{
				jumpIdx = i;
				for (int j = 0; j < jumpSymbolsCount; j++)
				{
					if (mText[i] == jumpSymbols[j])
					{
						stop = true;
						break;
					}
				}
			}

			if (!stop) jumpIdx++;
		}
		else
		{
			bool stop = false;
			for (int i = Math::Min(mSelectionEnd - 1, mText.Length() - 1); i >= 0 && !stop; i--)
			{
				jumpIdx = i;
				for (int j = 0; j < jumpSymbolsCount; j++)
				{
					if (mText[i] == jumpSymbols[j])
					{
						jumpIdx++;
						stop = true;
						break;
					}
				}
			}

			if (!stop) jumpIdx--;
		}

		if (jumpIdx == mSelectionEnd)
		{
			if (forward)
				jumpIdx++;
			else
				jumpIdx--;
		}

		mSelectionEnd = Math::Clamp(jumpIdx, 0, mText.Length());
		if (!selecting)
			mSelectionBegin = mSelectionEnd;

		mCaretBlinkTime = 0;

		UpdateSelectionAndCaret();
		CheckScrollingToCaret();
	}

	UInt16 GetUnicodeFromVirtualCode(KeyboardKey code)
	{
		HKL layout = GetKeyboardLayout(0);

		BYTE allKeys[256];
		GetKeyboardState(allKeys);

		UInt16 unicode;
		ToUnicodeEx(code, 0, allKeys, reinterpret_cast<wchar_t*>(&unicode), 1, 0, layout);
		return unicode;
	}

	void UIEditBox::CheckCharacterTyping(KeyboardKey key)
	{
		if (o2Input.IsKeyDown(VK_CONTROL))
			return;

		if (key == VK_ESCAPE)
			return;

		if (!mMultiLine && key == VK_RETURN)
			return;

		char16_t unicode = (char16_t)GetUnicodeFromVirtualCode(key);

		if (unicode != 0 && unicode != 8)
		{
			if (unicode == 13)
				unicode = 10;

			if (unicode == 10 && !mMultiLine)
				return;

			if (unicode == 9)
				return;

			if (mAvailableSymbols.Length() > 0)
			{
				bool found = false;
				for (int i = 0; i < mAvailableSymbols.Length(); i++)
				{
					if (mAvailableSymbols[i] == unicode)
					{
						found = true;
						break;
					}
				}

				if (!found)
					return;
			}

			if (mSelectionBegin != mSelectionEnd)
			{
				mText.Erase(Math::Min(mSelectionBegin, mSelectionEnd), Math::Max(mSelectionBegin, mSelectionEnd));
				mSelectionEnd = Math::Min(mSelectionBegin, mSelectionEnd);
				mSelectionBegin = mSelectionEnd;
			}

			mText.Insert(unicode, mSelectionEnd);
			mSelectionBegin = mSelectionEnd = mSelectionEnd + 1;
			mTextDrawable->SetText(mText);

			mCaretBlinkTime = 0;

			UpdateLayout();
			CheckScrollingToCaret();

			onChanged(mText);
		}
	}

	void UIEditBox::CheckCharactersErasing(KeyboardKey key)
	{
		if (key == VK_BACK)
		{
			if (mSelectionEnd == mSelectionBegin)
			{
				if (mSelectionEnd > 0)
					mText.Erase(mSelectionEnd - 1, mSelectionEnd);
			}
			else
			{
				mText.Erase(Math::Min(mSelectionBegin, mSelectionEnd), Math::Max(mSelectionEnd, mSelectionBegin));
			}

			mTextDrawable->SetText(mText);
			onChanged(mText);
			UpdateLayout();

			if (mSelectionEnd == mSelectionBegin)
				MoveCaret(mSelectionEnd - 1, false);
			else
				MoveCaret(Math::Min(mSelectionEnd, mSelectionBegin), false);
		}

		if (key == VK_DELETE)
		{
			if (mSelectionEnd < mText.Length() + 1)
			{
				if (mSelectionEnd == mSelectionBegin)
					mText.Erase(mSelectionEnd, mSelectionBegin + 1);
				else
					mText.Erase(Math::Min(mSelectionBegin, mSelectionEnd), Math::Max(mSelectionEnd, mSelectionBegin));

				mTextDrawable->SetText(mText);
				onChanged(mText);
				UpdateLayout();

				MoveCaret(Math::Min(mSelectionEnd, mSelectionBegin), false);
			}
		}
	}

	void UIEditBox::CheckCaretMoving(KeyboardKey key)
	{
		bool selecting = o2Input.IsKeyDown(VK_SHIFT);
		bool control = o2Input.IsKeyDown(VK_CONTROL);

		if (key == VK_LEFT)
		{
			if (control) JumpSelection(false, selecting);
			else MoveCaret(mSelectionEnd - 1, selecting);
		}

		if (key == VK_RIGHT)
		{
			if (control)
				JumpSelection(true, selecting);
			else
				MoveCaret(mSelectionEnd + 1, selecting);
		}

		if (key == 'A' && control)
			SelectAll();

		if (key == VK_UP)
			MoveCaret(GetTextCaretPosition(GetTextCaretPosition(mSelectionEnd) +
										   Vec2F(0.0f, mTextDrawable->GetFont()->GetLineHeight()*1.5f)), selecting);

		if (key == VK_DOWN)
			MoveCaret(GetTextCaretPosition(GetTextCaretPosition(mSelectionEnd) -
										   Vec2F(0.0f, mTextDrawable->GetFont()->GetLineHeight()*0.5f)), selecting);

		if (key == VK_END)
		{
			int endLineSymbol = mSelectionEnd;
			for (int i = mSelectionEnd; i < mText.Length(); i++)
			{
				endLineSymbol = i + 1;
				if (mText[i] == '\n')
				{
					endLineSymbol--;
					break;
				}
			}

			MoveCaret(endLineSymbol, selecting);
		}

		if (key == VK_HOME)
		{
			int endLineSymbol = mSelectionEnd;
			for (int i = mSelectionEnd - 1; i >= 0; i--)
			{
				endLineSymbol = i;
				if (mText[i] == '\n')
				{
					endLineSymbol++;
					break;
				}
			}

			MoveCaret(endLineSymbol, selecting);
		}
	}

	void UIEditBox::CheckClipboard(KeyboardKey key)
	{
		if (!o2Input.IsKeyDown(VK_CONTROL))
			return;

		if (key == 'C')
			Clipboard::SetText(mText.SubStr(Math::Min(mSelectionBegin, mSelectionEnd),
											Math::Max(mSelectionBegin, mSelectionEnd)));

		if (key == 'X')
		{
			Clipboard::SetText(mText.SubStr(Math::Min(mSelectionBegin, mSelectionEnd),
											Math::Max(mSelectionBegin, mSelectionEnd)));

			if (mSelectionBegin != mSelectionEnd)
			{
				mText.Erase(Math::Min(mSelectionBegin, mSelectionEnd), Math::Max(mSelectionBegin, mSelectionEnd));
				mSelectionBegin = mSelectionEnd = Math::Min(mSelectionBegin, mSelectionEnd);
			}

			mTextDrawable->SetText(mText);
			onChanged(mText);
			UpdateLayout();
			CheckScrollingToCaret();
		}

		if (key == 'V')
		{
			WString clipboard = Clipboard::GetText();

			if (mSelectionBegin != mSelectionEnd)
				mText.Erase(Math::Min(mSelectionBegin, mSelectionEnd), Math::Max(mSelectionBegin, mSelectionEnd));

			mSelectionEnd = Math::Min(mSelectionBegin, mSelectionEnd);
			mSelectionBegin = mSelectionEnd;

			mText.Insert(clipboard, mSelectionEnd);

			mSelectionBegin = mSelectionEnd = mSelectionEnd + clipboard.Length();

			mTextDrawable->SetText(mText);
			onChanged(mText);
			UpdateLayout();
			CheckScrollingToCaret();
		}
	}

	void UIEditBox::MoveCaret(int newPosition, bool selecting)
	{
		mSelectionEnd = Math::Clamp(newPosition, 0, mText.Length());
		if (!selecting)
			mSelectionBegin = mSelectionEnd;

		UpdateSelectionAndCaret();
		CheckScrollingToCaret();
		mCaretBlinkTime = 0;
	}

	void UIEditBox::InitializeProperties()
	{
		INITIALIZE_PROPERTY(UIEditBox, text, SetText, GetText);
		INITIALIZE_PROPERTY(UIEditBox, caret, SetCaretPosition, GetCaretPosition);
		INITIALIZE_PROPERTY(UIEditBox, selectionBegin, SetSelectionBegin, GetSelectionBegin);
		INITIALIZE_PROPERTY(UIEditBox, selectionEnd, SetSelectionEnd, GetSelectionEnd);
	}
}