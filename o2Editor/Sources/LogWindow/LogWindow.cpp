#include "LogWindow.h"

#include "Render/Sprite.h"
#include "UI/Button.h"
#include "UI/Label.h"
#include "UI/List.h"
#include "UI/LongList.h"
#include "UI/Toggle.h"
#include "UI/UIManager.h"

namespace Editor
{
	void LogWindow::Update(float dt)
	{
	}

	LogWindow::LogWindow():
		mRegularMessagesEnabled(true), mWarningMessagesEnabled(true), mErrorMessagesEnabled(true), mRegularMessagesCount(0),
		mWarningMessagesCount(0), mErrorMessagesCount(0)
	{
		InitializeWindow();
		BindStream(o2Debug.GetLog());
	}

	LogWindow::~LogWindow()
	{}

	void LogWindow::InitializeWindow()
	{
		mWindow->caption = "Log";
		mWindow->name = "log window";
		mWindow->SetIcon(mnew Sprite("ui/UI_list_icon.png"));
		mWindow->SetIconLayout(Layout::Based(BaseCorner::LeftTop, Vec2F(20, 20), Vec2F(-1, 1)));
		mWindow->SetViewLayout(Layout::BothStretch(-2, 0, 0, 18));
		mWindow->SetClippingLayout(Layout::BothStretch(-1, 0, 0, 18));

		mList = o2UI.CreateWidget<UILongList>();
		mList->layout = UIWidgetLayout::BothStretch(0, 18, 0, -1);
		mList->SetViewLayout(Layout::BothStretch());
		mList->getItemsCountFunc = THIS_FUNC(GetVisibleMessagesCount);
		mList->getItemsRangeFunc = THIS_FUNC(GetVisibleMessagesRange);
		mList->setupItemFunc = THIS_FUNC(SetupListMessage);

		UIWidget* listItemSample = mnew UIWidget();
		listItemSample->layout.minHeight = 25;
		listItemSample->AddLayer("back", mnew Sprite(Color4(212, 216, 224, 255)));
		listItemSample->AddLayer("warning", mnew Sprite(Color4(239, 196, 25, 255)),
								 Layout::VerStretch(HorAlign::Left, 0, 0, 10, 0));
		listItemSample->AddLayer("error", mnew Sprite(Color4(211, 87, 40, 255)),
								 Layout::VerStretch(HorAlign::Left, 0, 0, 10, 0));

		Text* captionText = mnew Text("stdFont.ttf");
		captionText->text = "text";
		captionText->horAlign = HorAlign::Left;
		captionText->verAlign = VerAlign::Middle;
		captionText->dotsEngings = true;
		listItemSample->AddLayer("caption", captionText, Layout::BothStretch(15, 0, 0, 0));

		mList->SetItemSample(listItemSample);

		mWindow->AddChild(mList);

		UIWidget* downPanel = mnew UIWidget();
		downPanel->AddLayer("back", mnew Sprite("ui/UI2_small_panel_down_back.png"),
							Layout::BothStretch(-4, -5, -4, -5));
		downPanel->layout = UIWidgetLayout::HorStretch(VerAlign::Bottom, 0, 0, 20, 0);
		mWindow->AddChild(downPanel);

		auto clearBtn = o2UI.CreateWidget<UIButton>("down panel trash");
		clearBtn->layout = UIWidgetLayout::Based(BaseCorner::Left, Vec2F(20, 20), Vec2F(0, 0));
		clearBtn->onClick = [&]() { OnClearPressed(); };
		downPanel->AddChild(clearBtn);

		auto messagesToggle = o2UI.CreateWidget<UIToggle>("log messages");
		messagesToggle->layout = UIWidgetLayout::Based(BaseCorner::Left, Vec2F(60, 20), Vec2F(20, 0));
		mMessagesCountLabel = messagesToggle->GetLayerDrawable<Text>("caption");
		mMessagesCountLabel->text = "0";
		messagesToggle->value = true;
		messagesToggle->onToggle = [&](bool value) { OnRegularMessagesToggled(value); };
		downPanel->AddChild(messagesToggle);

		auto warningsToggle = o2UI.CreateWidget<UIToggle>("log warnings");
		warningsToggle->layout = UIWidgetLayout::Based(BaseCorner::Left, Vec2F(60, 20), Vec2F(80, 0));
		mWarningsCountLabel = warningsToggle->GetLayerDrawable<Text>("caption");
		mWarningsCountLabel->text = "0";
		warningsToggle->value = true;
		warningsToggle->onToggle = [&](bool value) { OnWarningMessagesToggled(value); };
		downPanel->AddChild(warningsToggle);

		auto errorsToggle = o2UI.CreateWidget<UIToggle>("log errors");
		errorsToggle->layout = UIWidgetLayout::Based(BaseCorner::Left, Vec2F(60, 20), Vec2F(140, 0));
		mErrorsCountLabel = errorsToggle->GetLayerDrawable<Text>("caption");
		mErrorsCountLabel->text = "0";
		errorsToggle->value = true;
		errorsToggle->onToggle = [&](bool value) { OnErrorMessagesToggled(value); };
		downPanel->AddChild(errorsToggle);

		mLastMessageView = listItemSample->Clone();
		mLastMessageView->layout = UIWidgetLayout::BothStretch(200, 1, 0, 1);
		downPanel->AddChild(mLastMessageView);
		mLastMessageView->Hide(true);
	}

	void LogWindow::OnClearPressed()
	{
		mAllMessages.Clear();
		mVisibleMessages.Clear();
		mList->OnItemsUpdated();

		mRegularMessagesCount = 0;
		mWarningMessagesCount = 0;
		mErrorMessagesCount = 0;

		mMessagesCountLabel->text = "0";
		mWarningsCountLabel->text = "0";
		mErrorsCountLabel->text = "0";

		mLastMessageView->Hide(true);
	}

	void LogWindow::OnRegularMessagesToggled(bool value)
	{
		mRegularMessagesEnabled = value;
		UpdateVisibleMessages();
	}

	void LogWindow::OnWarningMessagesToggled(bool value)
	{
		mWarningMessagesEnabled = value;
		UpdateVisibleMessages();
	}

	void LogWindow::OnErrorMessagesToggled(bool value)
	{
		mErrorMessagesEnabled = value;
		UpdateVisibleMessages();
	}

	void LogWindow::UpdateVisibleMessages()
	{
		mVisibleMessages.Clear();

		for (auto& msg : mAllMessages)
		{
			msg.idx = mVisibleMessages.Count();

			if (msg.type == LogMessage::Type::Regular && mRegularMessagesEnabled)
				mVisibleMessages.Add(msg);

			if (msg.type == LogMessage::Type::Warning && mWarningMessagesEnabled)
				mVisibleMessages.Add(msg);

			if (msg.type == LogMessage::Type::Error && mErrorMessagesEnabled)
				mVisibleMessages.Add(msg);
		}

		mList->OnItemsUpdated(true);
	}

	int LogWindow::GetVisibleMessagesCount()
	{
		return mVisibleMessages.Count();
	}

	Vector<UnknownType*> LogWindow::GetVisibleMessagesRange(int min, int max)
	{
		Vector<UnknownType*> res;
		for (int i = min; i < max; i++)
		{
			if (i == mVisibleMessages.Count())
				break;

			res.Add((UnknownType*)(void*)&mVisibleMessages[i]);
		}

		return res;
	}

	void LogWindow::SetupListMessage(UIWidget* item, UnknownType* object)
	{
		LogMessage* message = (LogMessage*)(void*)object;

		item->layer["warning"]->drawable->enabled = message->type == LogMessage::Type::Warning;
		item->layer["error"]->drawable->enabled = message->type == LogMessage::Type::Error;
		item->layer["back"]->drawable->enabled = message->idx % 2 == 1;

		Text* text = item->GetLayerDrawable<Text>("caption");
		text->text = message->message.SubStr(0, message->message.Find("\n"));
	}

	void LogWindow::OutStrEx(const WString& str)
	{
		bool isScrollDown = Math::Equals(mList->GetScroll().y, mList->GetScrollRange().bottom, 5.0f) || true;

		LogMessage msg;
		msg.message = str;
		msg.type = LogMessage::Type::Regular;
		msg.idx = mAllMessages.Count();

		mAllMessages.Add(msg);
		if (mRegularMessagesEnabled)
		{
			msg.idx = mVisibleMessages.Count();
			mVisibleMessages.Add(msg);
		}

		mList->OnItemsUpdated();

		if (isScrollDown)
			mList->SetScrollForcible(Vec2F(0, mList->GetScrollRange().top));

		mRegularMessagesCount++;
		mMessagesCountLabel->text = (String)mRegularMessagesCount;

		UpdateLastMessageView();
	}

	void LogWindow::OutErrorEx(const WString& str)
	{
		bool isScrollDown = Math::Equals(mList->GetScroll().y, mList->GetScrollRange().bottom, 5.0f) || true;

		LogMessage msg;
		msg.message = str;
		msg.type = LogMessage::Type::Error;
		msg.idx = mAllMessages.Count();

		mAllMessages.Add(msg);
		if (mErrorMessagesEnabled)
		{
			msg.idx = mVisibleMessages.Count();
			mVisibleMessages.Add(msg);
		}

		mList->OnItemsUpdated();

		if (isScrollDown)
			mList->SetScrollForcible(Vec2F(0, mList->GetScrollRange().top));

		mErrorMessagesCount++;
		mErrorsCountLabel->text = (String)mErrorMessagesCount;

		UpdateLastMessageView();
	}

	void LogWindow::OutWarningEx(const WString& str)
	{
		bool isScrollDown = Math::Equals(mList->GetScroll().y, mList->GetScrollRange().bottom, 5.0f) || true;

		LogMessage msg;
		msg.message = str;
		msg.type = LogMessage::Type::Warning;
		msg.idx = mAllMessages.Count();

		mAllMessages.Add(msg);
		if (mWarningMessagesEnabled)
		{
			msg.idx = mVisibleMessages.Count();
			mVisibleMessages.Add(msg);
		}

		mList->OnItemsUpdated();

		if (isScrollDown)
			mList->SetScrollForcible(Vec2F(0, mList->GetScrollRange().top));

		mWarningMessagesCount++;
		mWarningsCountLabel->text = (String)mWarningMessagesCount;

		UpdateLastMessageView();
	}

	void LogWindow::UpdateLastMessageView()
	{
		if (mAllMessages.Count() > 0)
		{
			mLastMessageView->Show(true);
			SetupListMessage(mLastMessageView, (UnknownType*)(void*)&mAllMessages.Last());
		}
		else mLastMessageView->Hide(true);
	}

	bool LogWindow::LogMessage::operator==(const LogMessage& other) const
	{
		return type == other.type && message == other.message;
	}

}

CLASS_META(Editor::LogWindow)
{
	BASE_CLASS(Editor::IEditorWindow);
	BASE_CLASS(o2::LogStream);

	PROTECTED_FIELD(mList);
	PROTECTED_FIELD(mLastMessageView);
	PROTECTED_FIELD(mMessagesCountLabel);
	PROTECTED_FIELD(mWarningsCountLabel);
	PROTECTED_FIELD(mErrorsCountLabel);
	PROTECTED_FIELD(mAllMessages);
	PROTECTED_FIELD(mVisibleMessages);
	PROTECTED_FIELD(mRegularMessagesEnabled);
	PROTECTED_FIELD(mWarningMessagesEnabled);
	PROTECTED_FIELD(mErrorMessagesEnabled);
	PROTECTED_FIELD(mRegularMessagesCount);
	PROTECTED_FIELD(mWarningMessagesCount);
	PROTECTED_FIELD(mErrorMessagesCount);

	PUBLIC_FUNCTION(void, Update, float);
	PROTECTED_FUNCTION(void, InitializeWindow);
	PROTECTED_FUNCTION(void, OnClearPressed);
	PROTECTED_FUNCTION(void, OnRegularMessagesToggled, bool);
	PROTECTED_FUNCTION(void, OnWarningMessagesToggled, bool);
	PROTECTED_FUNCTION(void, OnErrorMessagesToggled, bool);
	PROTECTED_FUNCTION(void, UpdateVisibleMessages);
	PROTECTED_FUNCTION(int, GetVisibleMessagesCount);
	PROTECTED_FUNCTION(Vector<UnknownType*>, GetVisibleMessagesRange, int, int);
	PROTECTED_FUNCTION(void, SetupListMessage, UIWidget*, UnknownType*);
	PROTECTED_FUNCTION(void, OutStrEx, const WString&);
	PROTECTED_FUNCTION(void, OutErrorEx, const WString&);
	PROTECTED_FUNCTION(void, OutWarningEx, const WString&);
	PROTECTED_FUNCTION(void, UpdateLastMessageView);
}
END_META;

ENUM_META_(Editor::LogWindow::LogMessage::Type, Type)
{
	ENUM_ENTRY(Error);
	ENUM_ENTRY(Regular);
	ENUM_ENTRY(Warning);
}
END_ENUM_META;
