#ifndef MAGIQUE_GAME_EVENTS_H
#define MAGIQUE_GAME_EVENTS_H

#include <vector>
#include <magique/fwd.hpp>
#include <entt/entity/entity.hpp>

//===============================================
// Events
//===============================================
// ................................................................................
// The event module allows to emit any custom event and handle it in very flexible ways.
// It's different from the Scripting module in that you can have as many handlers as you want, and not limited to the entity type.
// Events can be gameplay related (player death, damage taken), technical (save loaded) or anything in between.
// A handler is just an implementation of the IEventHandler class.
// This is very powerful and simplifies the information flow throughout the game
//
// Note: You probably want to use a std::variants inside EventData to cleanly store typesafe data
// ................................................................................

enum class GameEvent : uint8_t; // User implemented to denote different events
struct EventData;               // User implemented event data

namespace magique
{
    struct IEventHandler
    {
        virtual ~IEventHandler() = default;

        // Called each time BEFORE any event method - event method is only called if returns true
        virtual bool shouldBeCalled() { return true; }

        // Called for each event
        virtual void onEvent(GameEvent event, entt::entity entity, const EventData& data) {}
    };

    // Returns the global event manager instance
    EventManager& GameEventsGet();

    struct EventManager final
    {
        // Adds an event handler
        //  - filter: only calls event func if emitted entity matches filter
        //  - priority: called in priority order descending - highest first
        EventSubID subscribe(IEventHandler* handler, entt::entity filter = {}, int priority = 0);

        // Returns true if the subscription has been removed
        bool unsubscribe(EventSubID id);

        // Immediately calls all handlers in correct order if conditions match (e.g. filter matches and shouldBeCalled() is true)
        // Template is only for cleaner signature - MUST be type EventData
        template <typename EventDataT = EventData>
        void emit(GameEvent event, entt::entity entity = entt::null, const EventDataT& data = {});

    private:
        struct EventSubscription final
        {
            bool isValid(entt::entity entity) const;
            IEventHandler* handler;
            entt::entity filter;
            EventSubID id;
            int priority;
            GameEvent event;
        };

        EventSubID curr = 0;
        std::vector<EventSubscription> subscribers;
    };

} // namespace magique

#endif // MAGIQUE_GAME_EVENTS_H
