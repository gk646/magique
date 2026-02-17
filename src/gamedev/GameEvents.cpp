#include <algorithm>

#include <magique/gamedev/GameEvents.h>

#include <magique/multiplayer/Networking.h>
#include <magique/ecs/ECS.h>

namespace magique
{
    inline EventManager EVENT_HANDLER{};

    EventManager& GameEventManager()
    {
        return EVENT_HANDLER;
    }

    EventSubID EventManager::subscribe(IEventHandler* handler, entt::entity filter, int priority)
    {
        auto& sub = subscribers.emplace_back();
        sub.handler = handler;
        sub.id = curr++;
        sub.filter = filter;
        sub.priority = priority;
        const EventSubID newSub = sub.id;
        std::ranges::sort(subscribers, [](const auto& one, const auto& two) { return one.priority > two.priority; });
        return newSub;
    }

    bool EventManager::unsubscribe(EventSubID id)
    {
        auto it = std::ranges::find_if(subscribers, [id](const EventSubscription& sub) { return sub.id == id; });
        if (it != subscribers.end())
        {
            // delete it->handler;
            *it = subscribers.back();
            subscribers.pop_back();
            return true;
        }
        else
        {
            return false;
        }
    }

    bool EventManager::EventSubscription::isValid(entt::entity entity) const
    {
        if (filter != entt::entity{} && filter != entity) [[unlikely]]
        {
            return false;
        }
        return true;
    }

} // namespace magique
