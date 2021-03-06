#include "ApplicationEventsListener.h"

#include "Events/EventSystem.h"

namespace o2
{
	ApplicationEventsListener::ApplicationEventsListener()
	{
		EventSystem::RegApplicationListener(this);
	}

	ApplicationEventsListener::~ApplicationEventsListener()
	{
		EventSystem::UnregApplicationListener(this);
	}

	void ApplicationEventsListener::OnApplicationStarted()
	{}

	void ApplicationEventsListener::OnApplicationClosing()
	{}

	void ApplicationEventsListener::OnApplicationActivated()
	{}

	void ApplicationEventsListener::OnApplicationDeactivated()
	{}

	void ApplicationEventsListener::OnApplicationSized()
	{}
}