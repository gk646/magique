// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_INTERNAL_SCRIPTING_H
#define MAGIQUE_INTERNAL_SCRIPTING_H

#include <magique/ecs/ECS.h>
#include <magique/internal/Macros.h>
M_IGNORE_WARNING(4100)

//===============================================
// Internal Scripting Module
//===============================================
// ................................................................................
// This module allows to create custom scripts for all entities of a type by subclassing EntityScript.
// This means it's a global instance for all entities of that type and MUST not contain any state.
// Different behavior is managed by a different component state of each entity (rather than state inside the script)
//
// Note: All entities are scripted per default - you need to explicitly disable this on a per-entity basis
// Note: You need to specify your custom type if you want to invoke a method that is specific to that type:
//      - MyEntityScript::onSit() -> InvokeEvent<onSit,MyEntityScript>(entity, place);
// It is possible to create behavior hierarchies where you rely on default behavior specified in the baseclass
//      -> see examples/headers/Scripting.h
//
// IMPORTANT: All event functions need their entity id as the first parameter!
// How to add and invoke your custom functions:
// Step 1: Create a new Subclass of EntityScript with a new function (or add it to the EntityScript here)
// Step 2: Add a new EventType enum with the name of your function
// Step 3: Add your new enum value to the REGISTER_EVENTS macro
// Step 4: Done! You can now invoke your event with: InvokeEvent<myEvent,MyEventScript>(self, args,...);
//
// Note: You can also create your own event system and only use the integrated events (or reroute them into your custom system)
// ................................................................................

namespace magique
{
    // Sets a script for this entity type
    // Subclass the EntityScript class and pass a new Instance()
    void ScriptingSetScript(EntityType entity, EntityScript* script);

    // Retrieves the script for the entity type
    // Failure: if no script is registered for the given type returns nullptr
    EntityScript* ScriptingGetScript(EntityType entity);

    // Sets the scripted status for the given entity - if set no automatic script methods will be called for this entity
    void ScriptingSetScripted(entt::entity entity, bool val);

    // Returns true if the given entity receives script updates
    bool ScriptingGetIsScripted(entt::entity entity);

    // Calls the given event function on the given entity
    // Note: If you want to access non-inherited methods you HAVE to pass your subclass type
    // IMPORTANT: 'arguments' are only parameters after the entity id - its passed implicitly
    // Examples:   InvokeEvent<onKeyEvent>(self);
    //             InvokeEvent<onItemPickup, MyPlayerScript>(self, item);
    //             InvokeEvent<onExplosion, MyGrenadeScript>(self, radius, damage);
    template <ScriptEvent event, class Script = EntityScript, class... Args>
    void ScriptingInvokeEvent(entt::entity entity, Args&&... arguments);

    // Same as 'InvokeEvent' but avoids the type lookup - very fast!
    template <ScriptEvent event, class Script = EntityScript, class... Args>
    void ScriptingInvokeEventDirect(EntityScript* script, entt::entity entity, Args&&... arguments);

    enum ScriptEvent : uint8_t
    {
        onCreate,
        onDestroy,
        onTick,
        onDynamicCollision,
        onStaticCollision,
    };

    // Add ALL event types here
    MQ_REGISTER_SCRIPT_EVENTS(onCreate, onDestroy, onTick, onDynamicCollision, onStaticCollision)

    struct EntityScript
    {
        virtual ~EntityScript() = default;

        //================= AUTOMATIC =================// // These events are called automatically

        // Called once after all components have been added
        virtual void onCreate(entt::entity self) {}

        // Called once before the entity is destroyed
        virtual void onDestroy(entt::entity self) {}

        // Called once at the beginning of each tick
        //      - updated: true if this entity is in update range of any actor (e.g. it's loaded)
        virtual void onTick(entt::entity self, bool updated) {}

        // Called each time this entity collides with another entity - called for both entities
        virtual void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info)
        {
            AccumulateCollision(info); // Treats the other shape as solid per default
        }

        // Called each time this entity collides with a static collision object
        virtual void onStaticCollision(entt::entity self, ColliderInfo collider, CollisionInfo& info)
        {
            AccumulateCollision(info); /// Treats the other shape as solid per default
        }

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

} // namespace magique

M_UNIGNORE_WARNING()

//================= IMPLEMENTATION =================//

namespace magique
{
    template <ScriptEvent event, class Script, class... Args>
    void magique::ScriptingInvokeEvent(entt::entity entity, Args&&... arguments)
    {
        const auto& pos = internal::REGISTRY.get<PositionC>(entity); // Every entity has a position
        auto* script = static_cast<Script*>(ScriptingGetScript(pos.type));
        MAGIQUE_ASSERT(script != nullptr, "No Script for this type!");
        Call<event, Script, entt::entity, Args...>(script, entity, std::forward<Args>(arguments)...);
    }

    template <ScriptEvent event, class Script, class... Args>
    void magique::ScriptingInvokeEventDirect(EntityScript* script, entt::entity entity, Args&&... arguments)
    {
        MAGIQUE_ASSERT(script != nullptr, "Passing a null script");
        Call<event, Script, entt::entity, Args...>(static_cast<Script*>(script), entity,
                                                   std::forward<Args>(arguments)...);
    }
} // namespace magique
#endif // MAGIQUE_INTERNAL_SCRIPTING_H
