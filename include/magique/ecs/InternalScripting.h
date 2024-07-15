#ifndef MAGIQUE_INTERNAL_SCRIPTING_H
#define MAGIQUE_INTERNAL_SCRIPTING_H

#include <magique/fwd.hpp>
#include <magique/ecs/ECS.h>
#include <magique/internal/Macros.h>

#pragma warning(push)
#pragma warning(disable : 4100) // unreferenced formal parameter

//-----------------------------------------------
// Internal Scripting Module
//-----------------------------------------------
// ................................................................................
// I thought quite a bit about how to do scripting. The main problem is that by
// allowing users to create their own components and functions its very hard
// to create a good scripting interface. This problem doesnt occur in other engines
// as they know all their components upfront and dont expose a lot of internal workings.
// This approach allows for vast, type safe! customization inside C++, while keeping
// the configuration manageable. Also this is still universal enough that supporting
// external scripting is still possible without changing the model.
// A script here is actually universal for all entities of that type and only exists once!
// All players have the same logic, all skeletons have the same base logic.
// Of course their component state can be different and they will behave differently
// ................................................................................

namespace magique
{
    enum EventType : uint8_t
    {
        onCreate,
        onDestroy,
        onDynamicCollision,
        onStaticCollision,
        onTick,
        onKeyEvent,
        onMouseEvent,
    };

    // Create a subclass of this to implemented specific behaviors
    // You can also create behavior hierarchies!

    // How to add your own functions!
    // IMPORTANT: All event functions need the registry and the self id as the first 2 parameters!
    // Step 1: Add a new EventType enum with the name of your function
    // Step 2: Add your new enum value to the REGISTER_EVENTS macro
    // Step 3: Done! You can now invoke your event!

    // Add ALL events here
    REGISTER_EVENTS(onCreate, onDestroy, onDynamicCollision, onStaticCollision, onTick, onKeyEvent, onMouseEvent);

    struct EntityScript
    {
        //----------------- AUTOMATIC -----------------// // These events are called automatically

        // Called once after all components have been added
        virtual void onCreate(entt::registry& registry, entt::entity self) {}

        // Called once before the entity is destroyed
        virtual void onDestroy(entt::registry& registry, entt::entity self) {}

        // Called when this entity collides with another entity - called for both entities
        virtual void onDynamicCollision(entt::registry& registry, entt::entity self, entt::entity other) {}

        // Called when this entity collides with a static collision object - walls...
        virtual void onStaticCollision(entt::registry& registry, entt::entity self) {}

        // Called at the beginning of each tick - only called on entities in update range
        virtual void onTick(entt::registry& registry, entt::entity self) {}

        // Called once at the beginning of each tick IF keystate changed - press or release
        virtual void onKeyEvent(entt::registry& registry, entt::entity self) {}

        // Called once at the beginning of each tick IF mouse state changed - includes mouse movement
        virtual void onMouseEvent(entt::registry& registry, entt::entity self) {}

        //----------------- USER -----------------// // These events have to be called by the user
        // Examples:

        // virtual void onMove(entt::registry& registry, entt::entity self, float newX, float newY) {}

        // virtual void onDeath(entt::registry& registry, entt::entity self, entt::entity killedBy) {}

        // virtual void onInteract(entt::registry& registry, entt::entity self, entt::entity target) {}

        // virtual void onItemPickup(entt::registry& registry, entt::entity self, Item& item) {}

        // virtual void onLevelUp(entt::registry& registry, entt::entity self) {}

        // ... feel free to add more global methods or create subclasses with special methods!
    };

    // Sets a c++ script for this entity type
    // Subclass the EntityScript class and pass a new Instance()
    // Note: Entities still need a ScriptC component to react to scripts! Use "GiveScript" when creating
    void SetScript(EntityID entity, EntityScript* script);

    // Retrieves the script for the entity type
    // Failure: returns nullptr
    EntityScript* GetScript(EntityID entity);

    // Calls the given event function on the given entity
    // Note: If you want to access non inherited methods you HAVE to pass your subclass type
    // IMPORTANT: 'arguments' are only parameters after the registry and self id - they are passed implicitly
    // Examples:   InvokeEvent<onKeyEvent>(self);
    //             InvokeEvent<onItemPickup, MyPlayerScript>(player, item);
    //             InvokeEvent<onExplosion, MyGrenadeScript>(grenade, radius, damage);
    template <EventType event, class Script = EntityScript, class... Args>
    void InvokeEvent(entt::entity entity, Args... arguments);

    // Same as 'InvokeEvent' but avoids the type lookup - very fast!
    template <EventType event, class Script = EntityScript, class... Args>
    void InvokeEventDirect(EntityScript* script, entt::entity entity, Args... arguments);

} // namespace magique

#pragma warning(pop)


// Implementation

template <magique::EventType event, class Script, class... Args>
void magique::InvokeEvent(entt::entity entity, Args... arguments)
{
    const auto& pos = REGISTRY.get<PositionC>(entity); // Every entity has a position
    auto* script = static_cast<Script*>(GetScript(pos.type));
    M_ASSERT(script != nullptr, "No Script for this type!");
    Call<event, Script, entt::registry&, entt::entity, Args...>(script, REGISTRY, entity,
                                                                std::forward<Args>(arguments)...);
}

template <magique::EventType event, class Script, class... Args>
void magique::InvokeEventDirect(EntityScript* script, entt::entity entity, Args... arguments)
{

    M_ASSERT(script != nullptr, "Passing a null script");
    Call<event, Script, entt::registry&, entt::entity, Args...>(static_cast<Script*>(script), REGISTRY, entity,
                                                                std::forward<Args>(arguments)...);
}

#endif //MAGIQUE_INTERNAL_SCRIPTING_H