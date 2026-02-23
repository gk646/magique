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
// Due to being typesafe the handling is a bit weird (as there is not reflection in c++ yet)
//
// Steps:
//      - 1: Make a new header and include GameEvents.h
//      - 2: Call the MQ_REGISTER_GAME_EVENTS enum with all the events you want to have
//      - 3: Define and implemented the event data types as you like
//      - 4: Done! Now you can emit the event
//
// How it works: For each enum you register:
//      - A enum value inside GameEvent is declared
//      - A type E[eventName]Data is declared (this data must be passed when emitted and is can be used when handling the event)
//      - A function on[eventName] is added to the IEventHandler class that can be implemented (and is called on that event)
//
// Note: The IEventHandler always contains a the shouldBeCalled() function - allows fine-grained control when event methods are called
// For more info see examples/headers/GameEvents.h
// Example header:
/*
#include <magique/gamedev/GameEvents.h>

// Define the possible events and register them
MQ_REGISTER_GAME_EVENTS(PlayerJoin, PlayerLeave);

// Define the data classes (named after the event [eventName]Data)
struct EPlayerJoinData
{
};

// Emit the event
magique::GameEventManager().emit<GameEvent::PlayerJoin>(client, EPlayerJoinData{});
*/
// Note: The implementation has to be included when using the Event system as it contains the type definitions
// ................................................................................

enum class GameEvent : uint8_t;

namespace magique
{
    // Always contains a function: virtual bool shouldBeCalled()
    // Is called each time BEFORE any event method - event method is only called if returns true
    struct IEventHandler;

    // Returns the global event manager instance
    EventManager& GameEventsGet();

    struct EventManager final
    {
        // Adds an event handler
        //  - filter: only calls event func if emitted entity matches filter
        //  - priority: called in priority order descending
        //  - hostOnly: only called when GetIsHost() is true
        EventSubID subscribe(IEventHandler* handler, entt::entity filter = {}, int priority = 0);

        // Returns true if the subscription has been removed
        bool unsubscribe(EventSubID id);

        // Immediately emits the event and calls all handler functions
        template <GameEvent event, typename Data>
        void emit(entt::entity entity, const Data& data = {});

        // Emit an event with optional entity and default initialized data
        template <GameEvent event>
        void emit(entt::entity entity = entt::null);

    private:
        struct EventSubscription final
        {
            bool isValid(entt::entity entity) const;

            IEventHandler* handler;
            entt::entity filter;
            EventSubID id;
            int16_t priority;
            GameEvent event;
            bool hostOnly;
        };

        EventSubID curr = 0;
        std::vector<EventSubscription> subscribers;
    };

} // namespace magique

#endif // MAGIQUE_GAME_EVENTS_H
