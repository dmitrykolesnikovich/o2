#include "DefaultActorHeaderViewer.h"

#include "PropertiesWindow/Properties/AssetProperty.h"
#include "PropertiesWindow/Properties/BooleanProperty.h"
#include "PropertiesWindow/Properties/LayerProperty.h"
#include "PropertiesWindow/Properties/StringProperty.h"
#include "PropertiesWindow/Properties/TagProperty.h"
#include "Scene/Actor.h"
#include "UI/Button.h"
#include "UI/DropDown.h"
#include "UI/EditBox.h"
#include "UI/Image.h"
#include "UI/Toggle.h"
#include "UI/Widget.h"

namespace Editor
{
	DefaultActorHeaderViewer::DefaultActorHeaderViewer()
	{
		mDataView = mnew UIWidget();
		mDataView->name = "actor head";
		mDataView->layout.minHeight = 42;

		mEnableProperty = mnew BooleanProperty(o2UI.CreateWidget<UIToggle>("actorHeadEnable"));
		mEnableProperty->GetWidget()->layout = UIWidgetLayout::Based(BaseCorner::LeftTop, Vec2F(20, 20), Vec2F(1, 0));
		mDataView->AddChild(mEnableProperty->GetWidget());

		mNameProperty = mnew StringProperty(o2UI.CreateWidget<UIEditBox>("actorHeadName"));
		mNameProperty->GetWidget()->layout = UIWidgetLayout::HorStretch(VerAlign::Top, 21, 25, 17, 2);
		mDataView->AddChild(mNameProperty->GetWidget());

		mLockProperty = mnew BooleanProperty(o2UI.CreateWidget<UIToggle>("actorHeadLock"));
		mLockProperty->GetWidget()->layout = UIWidgetLayout::Based(BaseCorner::RightTop, Vec2F(20, 20), Vec2F(-6, -1));
		mDataView->AddChild(mLockProperty->GetWidget());

		auto prototypeRoot = mDataView->AddChild(mnew UIWidget());
		prototypeRoot->name = "prototype";
		prototypeRoot->layout = UIWidgetLayout::BothStretch();
		prototypeRoot->AddState("visible", Animation::EaseInOut(prototypeRoot, &prototypeRoot->transparency, 0.0f, 1.0f, 0.1f));

		auto linkImg = o2UI.CreateImage("ui/UI2_prefab_link_big.png");
		linkImg->layout = UIWidgetLayout::Based(BaseCorner::LeftTop, Vec2F(20, 20), Vec2F(1, -20));
		prototypeRoot->AddChild(linkImg);

		mPrototypeProperty = mnew AssetProperty<ActorAssetRef>(o2UI.CreateWidget<UIWidget>("actorHeadAssetProperty"));
		mPrototypeProperty->GetWidget()->layout = UIWidgetLayout::HorStretch(VerAlign::Top, 21, 70, 17, 22);
		prototypeRoot->AddChild(mPrototypeProperty->GetWidget());

		mPrototypeApplyBtn = o2UI.CreateWidget<UIButton>("acceptPrototype");
		mPrototypeApplyBtn->layout = UIWidgetLayout::Based(BaseCorner::RightTop, Vec2F(25, 25), Vec2F(-46, -18));
		prototypeRoot->AddChild(mPrototypeApplyBtn);

		mPrototypeRevertBtn = o2UI.CreateWidget<UIButton>("revertPrototype");
		mPrototypeRevertBtn->layout = UIWidgetLayout::Based(BaseCorner::RightTop, Vec2F(25, 25), Vec2F(-26, -18));
		prototypeRoot->AddChild(mPrototypeRevertBtn);

		mPrototypeBreakBtn = o2UI.CreateWidget<UIButton>("breakPrototype");
		mPrototypeBreakBtn->layout = UIWidgetLayout::Based(BaseCorner::RightTop, Vec2F(25, 25), Vec2F(-6, -18));
		prototypeRoot->AddChild(mPrototypeBreakBtn);

		auto tagsImg = o2UI.CreateImage("ui/UI2_tag_big.png");
		tagsImg->layout = UIWidgetLayout::Based(BaseCorner::LeftBottom, Vec2F(20, 20), Vec2F(1, 1));
		mDataView->AddChild(tagsImg);

		mTagsProperty = mnew TagsProperty(o2UI.CreateWidget<UIEditBox>("actorHeadTags"));
		mTagsProperty->GetWidget()->layout = UIWidgetLayout::HorStretch(VerAlign::Bottom, 21, 135, 17, 3);
		mDataView->AddChild(mTagsProperty->GetWidget());

		auto layerImg = o2UI.CreateImage("ui/UI2_layer_big.png");
		layerImg->layout = UIWidgetLayout::Based(BaseCorner::RightBottom, Vec2F(20, 20), Vec2F(-115, 1));
		mDataView->AddChild(layerImg);

		mLayerProperty = mnew LayerProperty(o2UI.CreateWidget<UIDropDown>("actorHeadLayer"));
		mLayerProperty->GetWidget()->layout = UIWidgetLayout::Based(BaseCorner::RightBottom, Vec2F(106, 17), Vec2F(-10, 3));
		mDataView->AddChild(mLayerProperty->GetWidget());

		Animation protoStateAnim = Animation::EaseInOut(mDataView, &mDataView->layout.minHeight, 42.0f, 62.0f, 0.1f);
		*protoStateAnim.AddAnimationValue(&prototypeRoot->visible) = AnimatedValue<bool>::Linear(false, true, 0.1f);
		mDataView->AddState("prototype", protoStateAnim);
	}

	DefaultActorHeaderViewer::~DefaultActorHeaderViewer()
	{
		delete mEnableProperty;
		delete mDataView;
	}

	void DefaultActorHeaderViewer::SetTargetActors(const Vector<Actor*>& actors)
	{
		Vector<void*> enableTargets = actors.Select<void*>([](Actor* x) { return &x->enabled; });
		mEnableProperty->Setup(enableTargets, true);

		Vector<void*> nameTargets = actors.Select<void*>([](Actor* x) { return &x->name; });
		mNameProperty->Setup(nameTargets, true);

		Vector<void*> lockTargets = actors.Select<void*>([](Actor* x) { return &x->locked; });
		mLockProperty->Setup(lockTargets, true);

 		Vector<void*> prototypeTargets = actors.Select<void*>([](Actor* x) { return &x->prototype; });
 		mPrototypeProperty->Setup(prototypeTargets, true);

		Vector<void*> tagsTargets = actors.Select<void*>([](Actor* x) { return &x->tags; });
		mTagsProperty->Setup(tagsTargets, false);

		Vector<void*> layersTargets = actors.Select<void*>([](Actor* x) { return &x->layer; });
		//mLayerProperty->Setup(tagsTargets, true);
	}

	UIWidget* DefaultActorHeaderViewer::GetWidget() const
	{
		return mDataView;
	}

	void DefaultActorHeaderViewer::OnKeyPressed(const Input::Key& key)
	{
		if (key == 'P')
			*mDataView->state["prototype"] = !*mDataView->state["prototype"];
	}

}
 
CLASS_META(Editor::DefaultActorHeaderViewer)
{
	BASE_CLASS(Editor::IActorHeaderViewer);
	BASE_CLASS(o2::KeyboardEventsListener);

	PUBLIC_FIELD(mDataView);
	PUBLIC_FIELD(mEnableProperty);
	PUBLIC_FIELD(mNameProperty);
	PUBLIC_FIELD(mLockProperty);
	PUBLIC_FIELD(mPrototypeProperty);
	PUBLIC_FIELD(mPrototypeApplyBtn);
	PUBLIC_FIELD(mPrototypeRevertBtn);
	PUBLIC_FIELD(mPrototypeBreakBtn);
	PUBLIC_FIELD(mTagsProperty);
	PUBLIC_FIELD(mLayerProperty);

	PUBLIC_FUNCTION(void, SetTargetActors, const Vector<Actor*>&);
	PUBLIC_FUNCTION(UIWidget*, GetWidget);
	PROTECTED_FUNCTION(void, OnKeyPressed, const Input::Key&);
}
END_META;
 