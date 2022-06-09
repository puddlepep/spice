#include "EventHandler.hpp"

namespace EventHandler
{

	Event::Event(EventType _type, void* _data):
		Type(_type), Data(_data) {}

	Event::Event():
		Type(EventType::None), Data(nullptr) {}

	Event PullEvent(EventHandleType _handleType)
	{
		if (_handleType == EventHandleType::EngineHandled)
		{
			Event e = EngineHandledEvents[0];
			EngineHandledEvents.erase(EngineHandledEvents.begin());
			return e;
		}

		return Event();
	}

	void PushEvent(EventHandleType _handleType, EventType _eventType, void* _data)
	{
		if (_handleType == EventHandleType::EngineHandled)
			EngineHandledEvents.push_back(Event(_eventType, _data));
	}

	int GetEventCount(EventHandleType _handleType)
	{
		if (_handleType == EventHandleType::EngineHandled)
			return EngineHandledEvents.size();

		return 0;
	}
}
