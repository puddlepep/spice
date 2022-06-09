#pragma once

#include <vector>
#include <string>

namespace EventHandler
{

	enum class EventHandleType
	{
		EngineHandled,
	};

	enum class EventType
	{
		None,
		AddObject,
		DestroyObject,
	};

	struct Event
	{
		EventType Type;
		void* Data;

		Event(EventType _type, void* _data);
		Event();
	};

	static std::vector<Event> EngineHandledEvents;

	Event PullEvent(EventHandleType _handleType);
	void PushEvent(EventHandleType _handleType, EventType _eventType, void* _data);
	int GetEventCount(EventHandleType _handleType);
	
}