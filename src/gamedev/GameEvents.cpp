#include <algorithm>

#include <magique/gamedev/GameEvents.h>

#include <magique/multiplayer/Networking.h>
#include <magique/ecs/ECS.h>

namespace magique
{
    inline EventManager EVENT_HANDLER{};

    EventManager& GameEventsGet() { return EVENT_HANDLER; }

    EventSubID EventManager::subscribe(IEventHandler* handler, entt::entity filter, int priority)
    {
        auto& sub = subscribers.emplace_back();
        sub.handler = handler;
        sub.id = curr;
        sub.filter = filter;
        sub.priority = priority;
        std::ranges::sort(subscribers, [](const auto& one, const auto& two) { return one.priority > two.priority; });
        return curr++;
    }

    EventSubID EventManager::subscribe(const EventFunc& func, entt::entity filter, int priority)
    {
        struct Handler final : IEventHandler
        {
            Handler(const EventFunc& func) : func(func) {}
            void onEvent(GameEvent event, entt::entity entity, const EventData& data) override
            {
                func(event, entity, data);
            }
            EventFunc func;
        };
        return subscribe(new Handler(func), filter, priority);
    }

    bool EventManager::unsubscribe(EventSubID id)
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

    template <>
    void EventManager::emit(GameEvent event, entt::entity entity, const EventData& data)
    {
        for (auto& subscriber : subscribers)
        {
            if (subscriber.isValid(entity) && subscriber.handler->shouldBeCalled())
            {
                subscriber.handler->onEvent(event, entity, data);
            }
        }
    }

    bool EventManager::EventSubscription::isValid(entt::entity entity) const
    {
        if (filter != entt::null && filter != entity) [[unlikely]]
        {
            return false;
        }
        return true;
    }


} // namespace magique
