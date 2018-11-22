#include <dali/internal/window-system/windows/event-system-win.h>
#include <map>

using namespace std;

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

struct TCallbackInfo
{
    TCallbackInfo(EventCallback callback, void* handler)
    {
        this->callback = callback;
        this->handler = handler;
    }

    EventCallback callback;
    void* handler;
};

map<int, TCallbackInfo*> eventTypeToCallbackInfo;

Event_Handler event_handler_add(int eventType, EventCallback callback, void* handler)
{
    eventTypeToCallbackInfo.insert(make_pair(eventType, new TCallbackInfo(callback, handler)));
    return eventType;
}

void event_handler_del(Event_Handler handler)
{
    map<int, TCallbackInfo*>::iterator x = eventTypeToCallbackInfo.find(handler);

    if (eventTypeToCallbackInfo.end() != x)
    {
        delete x->second;
        eventTypeToCallbackInfo.erase( handler );
    }
}

void* GetEventHandler(int eventType)
{
    map<int, TCallbackInfo*>::iterator x = eventTypeToCallbackInfo.find(eventType);

    if (eventTypeToCallbackInfo.end() != x)
    {
        return x->second->handler; 
    }
    else
    {
        return NULL;
    }
}

EventCallback GetCallback(int eventType)
{
    map<int, TCallbackInfo*>::iterator x = eventTypeToCallbackInfo.find(eventType);

    if (eventTypeToCallbackInfo.end() != x)
    {
        return x->second->callback;
    }
    else
    {
        return NULL;
    }
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali