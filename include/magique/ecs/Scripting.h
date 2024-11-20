#ifndef MAGIQUE_INTERNAL_SCRIPTING_H
#define MAGIQUE_INTERNAL_SCRIPTING_H

#include <magique/ecs/ECS.h>
#include <magique/internal/Macros.h>
IGNORE_WARNING(4100)

//===============================================
// Internal Scripting Module
//===============================================
// ................................................................................
// I thought quite a bit about how to do scripting. The main problem is that by
// allowing users to create their own components and functions it's very hard
// to create a good scripting interface. This problem doesn't occur in other engines
// as they know all their components upfront and don't expose a lot of internal workings.
// This approach allows for vast, type safe! Customization inside C++, while keeping
// the configuration manageable. Also, this is still universal enough that supporting
// external scripting is still possible without changing the model.
// A script here is actually global for all entities of that type and only exists once!
// All players have the same logic, all skeletons have the same base logic.
// Of course their component state can be different, and they will behave differently...
// ................................................................................

namespace magique
{
    enum EventType : uint8_t
    {
        onCreate,
        onDestroy,
        onTick,
        onDynamicCollision,
        onStaticCollision,
        onKeyEvent,
        onMouseEvent,
    };

    // Create a subclass of this to implemented specific behaviors
    // You can also create behavior hierarchies!

    // How to add your own functions!
    // IMPORTANT: All event functions need the self id as the first parameter!
    // Step 1: Add a new EventType enum with the name of your function
    // Step 2: Add your new enum value to the REGISTER_EVENTS macro
    // Step 3: Done! You can now invoke your event!

    // Add ALL events here
    REGISTER_EVENTS(onCreate, onDestroy, onTick, onDynamicCollision, onStaticCollision, onKeyEvent, onMouseEvent);

    struct EntityScript
    {
        //================= AUTOMATIC =================// // These events are called automatically

        // Called once after all components have been added
        virtual void onCreate(entt::entity self) {}

        // Called once before the entity is destroyed
        virtual void onDestroy(entt::entity self) {}

        // Called once at the beginning of each tick - only called on updated entities (in update range or cache)
        virtual void onTick(entt::entity self) {}

        // Called each time this entity collides with another entity - called for both entities
        virtual void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info)
        {
            AccumulateCollision(info); // Treats the other shape as solid per default
        }

        // Called each time when this entity collides with a static collision object
        virtual void onStaticCollision(entt::entity self, ColliderInfo collider, CollisionInfo& info)
        {
            AccumulateCollision(info); /// Treats the other shape as solid per default
        }

        // Called once at the beginning of each tick IF key state changed - press or release
        virtual void onKeyEvent(entt::entity self) {}

        // Called once at the beginning of each tick IF mouse state changed - includes mouse movement
        virtual void onMouseEvent(entt::entity self) {}

        //================= USER =================// // These events have to be called by the user
        // Examples:

        // virtual void onInteract(entt::entity self, entt::entity target) {}

        // virtual void onItemPickup(entt::entity self, Item& item) {}

        // ... feel free to add more global methods or create subclasses with special methods!

        //================= UTIL =================//

        // Adds the given info on top the existing info for this entity - will be applied after all collisions are resolved
        // Note: This essentially makes the other shape 'solid' preventing you from entering it!
        static void AccumulateCollision(CollisionInfo& collisionInfo);
    };

    // Sets a C++ script for this entity type
    // Subclass the EntityScript class and pass a new Instance()
    // Note: Entities still need a ScriptC component to react to scripts! Use "GiveScript" when creating
    void SetEntityScript(EntityType entity, EntityScript* script);

    // Retrieves the script for the entity type
    // Failure: returns nullptr
    EntityScript* GetEntityScript(EntityType entity);

    // Calls the given event function on the given entity
    // Note: If you want to access non-inherited methods you HAVE to pass your subclass type
    // IMPORTANT: 'arguments' are only parameters after the self id - its passed implicitly
    // Examples:   InvokeEvent<onKeyEvent>(self);
    //             InvokeEvent<onItemPickup, MyPlayerScript>(self, item);
    //             InvokeEvent<onExplosion, MyGrenadeScript>(self, radius, damage);
    template <EventType event, class Script = EntityScript, class... Args>
    void InvokeEvent(entt::entity entity, Args&&... arguments);

    // Same as 'InvokeEvent' but avoids the type lookup - very fast!
    template <EventType event, class Script = EntityScript, class... Args>
    void InvokeEventDirect(EntityScript* script, entt::entity entity, Args&&... arguments);

} // namespace magique

UNIGNORE_WARNING()

//================= IMPLEMENTATION =================//

namespace magique
{
    template <EventType event, class Script, class... Args>
    void magique::InvokeEvent(entt::entity entity, Args&&... arguments)
    {
        const auto& pos = internal::REGISTRY.get<PositionC>(entity); // Every entity has a position
        auto* script = static_cast<Script*>(GetEntityScript(pos.type));
        MAGIQUE_ASSERT(script != nullptr, "No Script for this type!");
        Call<event, Script, entt::entity, Args...>(script, entity, std::forward<Args>(arguments)...);
    }
    template <EventType event, class Script, class... Args>
    void magique::InvokeEventDirect(EntityScript* script, entt::entity entity, Args&&... arguments)
    {
        MAGIQUE_ASSERT(script != nullptr, "Passing a null script");
        Call<event, Script, entt::entity, Args...>(static_cast<Script*>(script), entity,
                                                   std::forward<Args>(arguments)...);
    }
} // namespace magique
#endif //MAGIQUE_INTERNAL_SCRIPTING_H