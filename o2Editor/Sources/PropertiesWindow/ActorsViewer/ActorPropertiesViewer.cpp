#include "ActorPropertiesViewer.h"

#include "PropertiesWindow/ActorsViewer/DefaultActorAnimationViewer.h"
#include "PropertiesWindow/ActorsViewer/DefaultActorComponentViewer.h"
#include "PropertiesWindow/ActorsViewer/DefaultActorHeaderViewer.h"
#include "PropertiesWindow/ActorsViewer/DefaultActorTransformViewer.h"
#include "PropertiesWindow/ActorsViewer/IActorAnimationViewer.h"
#include "PropertiesWindow/ActorsViewer/IActorComponentViewer.h"
#include "PropertiesWindow/ActorsViewer/IActorHeaderViewer.h"
#include "PropertiesWindow/ActorsViewer/IActorTransformViewer.h"
#include "Scene/Actor.h"
#include "UI/ScrollArea.h"
#include "UI/UIManager.h"
#include "UI/VerticalLayout.h"

namespace Editor
{
	ActorPropertiesViewer::ActorPropertiesViewer()
	{
		mHeaderViewer = mnew DefaultActorHeaderViewer();
		mTransformViewer = mnew DefaultActorTransformViewer();
		mAnimationViewer = mnew DefaultActorAnimationViewer();
		mDefaultComponentViewer = mnew DefaultActorComponentViewer();

		auto componentsViewersTypes = TypeOf(IActorComponentViewer).GetDerivedTypes();
		for (auto type : componentsViewersTypes)
			mAvailableComponentsViewers.Add((IActorComponentViewer*)type->CreateSample());

		auto scrollArea = o2UI.CreateScrollArea("backless");
		scrollArea->SetViewLayout(Layout::BothStretch());
		scrollArea->SetClippingLayout(Layout::BothStretch());
		scrollArea->name = "actors scroll area";
		mContentWidget = scrollArea;

		mViewersLayout = o2UI.CreateVerLayout();
		mViewersLayout->name          = "viewers layout";
		mViewersLayout->spacing       = 0.0f;
		mViewersLayout->border        = RectF();
		mViewersLayout->expandHeight  = false;
		mViewersLayout->expandWidth   = true;
		mViewersLayout->fitByChildren = true;
		mViewersLayout->baseCorner    = BaseCorner::Top;
		mViewersLayout->layout        = UIWidgetLayout::BothStretch();
		mContentWidget->AddChild(mViewersLayout);

		o2Scene.onChanged += [&](Vector<Actor*>) { Refresh(); };
	}

	ActorPropertiesViewer::~ActorPropertiesViewer()
	{
		for (auto kv : mComponentViewersPool)
		{
			for (auto x : kv.Value())
				delete x;
		}

		for (auto x : mAvailableComponentsViewers)
			delete x;

		delete mDefaultComponentViewer;
		delete mHeaderViewer;
		delete mTransformViewer;
		delete mAnimationViewer;
	}

	const Type* ActorPropertiesViewer::GetViewingObjectType() const
	{
		return &TypeOf(Actor);
	}

	void ActorPropertiesViewer::SetActorHeaderViewer(IActorHeaderViewer* viewer)
	{
		delete mHeaderViewer;
		mHeaderViewer = viewer;
	}

	void ActorPropertiesViewer::SetActorTransformViewer(IActorTransformViewer* viewer)
	{
		delete mTransformViewer;
		mTransformViewer = viewer;
	}

	void ActorPropertiesViewer::SetActorAnimationViewer(IActorAnimationViewer* viewer)
	{
		delete mAnimationViewer;
		mAnimationViewer = viewer;
	}

	void ActorPropertiesViewer::AddComponentViewerType(IActorComponentViewer* viewer)
	{
		mAvailableComponentsViewers.Add(viewer);
	}

	void ActorPropertiesViewer::Refresh()
	{
		if (mTargetActors.IsEmpty())
			return;

		for (auto viewer : mComponentsViewers)
			viewer->Refresh();

		mTransformViewer->Refresh();
		mHeaderViewer->Refresh();
	}

	void ActorPropertiesViewer::SetTargets(const Vector<IObject*> targets)
	{
		mTargetActors = targets.Select<Actor*>([](auto x) { return (Actor*)x; });

		// clear 
		mViewersLayout->RemoveAllChilds(false);

		for (auto viewer : mComponentsViewers)
			mComponentViewersPool[viewer->GetComponentType()].Add(viewer);

		mComponentsViewers.Clear();

		// rebuild
		Vector<UIWidget*> viewersWidgets;
		viewersWidgets.Add(mHeaderViewer->GetWidget());
		mHeaderViewer->SetTargetActors(mTargetActors);

		viewersWidgets.Add(mTransformViewer->GetWidget());
		mTransformViewer->SetTargetActors(mTargetActors);

		viewersWidgets.Add(mAnimationViewer->GetWidget());
		mAnimationViewer->SetTargetActors(mTargetActors);

		// components
		auto commonComponentsTypes = mTargetActors[0]->GetComponents().Select<const Type*>([](auto x) {
			return &x->GetType(); });

		for (int i = 1; i < mTargetActors.Count(); i++)
		{
			auto actorComponentsTypes = mTargetActors[i]->GetComponents().Select<const Type*>([](auto x) {
				return &x->GetType(); });

			auto commTypesTemp = commonComponentsTypes;
			for (auto type : commTypesTemp)
			{
				if (!actorComponentsTypes.Contains(type))
					commonComponentsTypes.Remove(type);
			}
		}

		for (const Type* type : commonComponentsTypes)
		{
			bool usingDefaultComponentViewer = false;

			auto viewerSample = mAvailableComponentsViewers.FindMatch([&](IActorComponentViewer* x) {
				return x->GetComponentType() == type; });

			if (!viewerSample)
			{
				viewerSample = mDefaultComponentViewer;
				usingDefaultComponentViewer = true;
			}

			if (!mComponentViewersPool.ContainsKey(type) || mComponentViewersPool[type].IsEmpty())
			{
				if (!mComponentViewersPool.ContainsKey(type))
					mComponentViewersPool.Add(type, Vector<IActorComponentViewer*>());

				auto newViewer = (IActorComponentViewer*)(viewerSample->GetType().CreateSample());

				if (usingDefaultComponentViewer)
					((DefaultActorComponentViewer*)newViewer)->SepcializeComponentType(type);

				mComponentViewersPool[type].Add(newViewer);
			}

			auto componentViewer = mComponentViewersPool[type].PopBack();

			viewersWidgets.Add(componentViewer->GetWidget());
			mComponentsViewers.Add(componentViewer);

			componentViewer->SetTargetComponents(
				mTargetActors.Select<Component*>([&](Actor* x) { return x->GetComponent(type); }));
		}

		mViewersLayout->AddChilds(viewersWidgets);
	}

	void ActorPropertiesViewer::OnEnabled()
	{}

	void ActorPropertiesViewer::OnDisabled()
	{
		mTargetActors.Clear();
	}

	void ActorPropertiesViewer::Update(float dt)
	{}

	void ActorPropertiesViewer::Draw()
	{}

}

CLASS_META(Editor::ActorPropertiesViewer)
{
	BASE_CLASS(Editor::IObjectPropertiesViewer);

	PROTECTED_FIELD(mTargetActors);
	PROTECTED_FIELD(mHeaderViewer);
	PROTECTED_FIELD(mTransformViewer);
	PROTECTED_FIELD(mAnimationViewer);
	PROTECTED_FIELD(mComponentsViewers);
	PROTECTED_FIELD(mDefaultComponentViewer);
	PROTECTED_FIELD(mAvailableComponentsViewers);
	PROTECTED_FIELD(mComponentViewersPool);
	PROTECTED_FIELD(mViewersLayout);

	PUBLIC_FUNCTION(const Type*, GetViewingObjectType);
	PUBLIC_FUNCTION(void, SetActorHeaderViewer, IActorHeaderViewer*);
	PUBLIC_FUNCTION(void, SetActorTransformViewer, IActorTransformViewer*);
	PUBLIC_FUNCTION(void, SetActorAnimationViewer, IActorAnimationViewer*);
	PUBLIC_FUNCTION(void, AddComponentViewerType, IActorComponentViewer*);
	PUBLIC_FUNCTION(void, Refresh);
	PROTECTED_FUNCTION(void, SetTargets, const Vector<IObject*>);
	PROTECTED_FUNCTION(void, OnEnabled);
	PROTECTED_FUNCTION(void, OnDisabled);
	PROTECTED_FUNCTION(void, Update, float);
	PROTECTED_FUNCTION(void, Draw);
}
END_META;
