#include <algorithm>

#include <magique/gamedev/GameEvents.h>

#include <magique/multiplayer/Networking.h>
#include <magique/ecs/ECS.h>

namespace magique
{

    struct EventSubData final
    {
        bool isValid(Entity entity) const
        {
            if (filter != entt::null && filter != entity) [[unlikely]]
            {
                return false;
            }
            return true;
        }

        IEventHandler* handler;
        int priority;
        Entity filter;
        EventSubscription id;
    };

    static EventSubscription curr = 1;
    static std::vector<EventSubData> subscribers;

    template <>
    void EventsEmit(Event event, Entity entity, const EventData& data)
    {
        for (auto& subscriber : subscribers)
        {
            if (subscriber.isValid(entity) && subscriber.handler->shouldBeCalled(event, entity, data))
            {
                subscriber.handler->onEvent(event, entity, data);
            }
        }
    }

    EventSubscription EventsSubscribe(IEventHandler* handler, Entity filter, int priority)
    {
        auto& sub = subscribers.emplace_back();
        sub.handler = handler;
        sub.id = curr;
        sub.filter = filter;
        sub.priority = priority;
        std::ranges::sort(subscribers, [](const auto& one, const auto& two) { return one.priority > two.priority; });
        return curr++;
    }

    EventSubscription EventsSubscribe(const EventFunc& func, Entity filter, int priority)
    {
        struct Handler final : IEventHandler
        {
            Handler(const EventFunc& func) : func(func) {}
            void onEvent(Event event, Entity entity, const EventData& data) override { func(event, entity, data); }
            EventFunc func;
        };
        return EventsSubscribe(new Handler(func), filter, priority);
    }

    bool EventsCancel(EventSubscription id)
    {
        if (id == 0)
        {
            return false;
        }
        return std::erase_if(subscribers,
                             [id](const auto& sub)
                             {
                                 if (sub.id == id)
                                 {
                                     delete sub.handler;
                                     return true;
                                 }
                                 return false;
                             }) > 0;
    }

} // namespace magique
