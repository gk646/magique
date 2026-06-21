#include <algorithm>

#include <magique/gamedev/GameEvents.h>

#include <magique/multiplayer/Networking.h>
#include <magique/ecs/ECS.h>

namespace magique
{

    struct EventSubscription final
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
        EventSubID id;
        GameEvent event;
    };

    static EventSubID curr = 1;
    static std::vector<EventSubscription> subscribers;

    template <>
    void GameEventsEmit(GameEvent event, Entity entity, const EventData& data)
    {
        for (auto& subscriber : subscribers)
        {
            if (subscriber.isValid(entity) && subscriber.handler->shouldBeCalled(event, entity, data))
            {
                subscriber.handler->onEvent(event, entity, data);
            }
        }
    }

    EventSubID GameEventsSubscribe(IEventHandler* handler, Entity filter, int priority)
    {
        auto& sub = subscribers.emplace_back();
        sub.handler = handler;
        sub.id = curr;
        sub.filter = filter;
        sub.priority = priority;
        std::ranges::sort(subscribers, [](const auto& one, const auto& two) { return one.priority > two.priority; });
        return curr++;
    }

    EventSubID GameEventsSubscribe(const EventFunc& func, Entity filter, int priority)
    {
        struct Handler final : IEventHandler
        {
            Handler(const EventFunc& func) : func(func) {}
            void onEvent(GameEvent event, Entity entity, const EventData& data) override { func(event, entity, data); }
            EventFunc func;
        };
        return GameEventsSubscribe(new Handler(func), filter, priority);
    }

    bool GameEventsCancel(EventSubID id)
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
