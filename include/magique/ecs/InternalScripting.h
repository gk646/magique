#ifndef MAGIQUE_INTERNAL_SCRIPTING_H
#define MAGIQUE_INTERNAL_SCRIPTING_H

#include <cassert>
#include <magique/fwd.hpp>
#include <magique/ecs/Registry.h>
#include <magique/util/Macros.h>

//-----------------------------------------------
// Internal Scripting Module
//-----------------------------------------------
// .....................................................................
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
// .....................................................................

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
        myCustomEvent,
    };

    // Create a subclass of this to implemented specific behaviors
    // You can also create behavior hierarchies!

    // How to add your own functions!
    // IMPORTANT: All event functions need the registry and the self id as the first 2 parameters!
    // Step 1: Add a new EventType enum with the name of your function
    // Step 2: Copy and insert a HANDLE_EVENT macro with your new name inside  getEventFunction()
    // Step 3: Add your function with the same name and your done!

    struct EntityScript
    {
        template <EventType event, typename... Args>
        static constexpr EventFunc<Args...> getEventFunction()
        {
            HANDLE_EVENT(onCreate);
            HANDLE_EVENT(onDestroy);
            HANDLE_EVENT(onDynamicCollision);
            HANDLE_EVENT(onStaticCollision);
            HANDLE_EVENT(onTick);
            HANDLE_EVENT(onKeyEvent);
            HANDLE_EVENT(onMouseEvent);
            // Add your custom events here - enum and function name have to match
            // And you have to pass the correct arguments when calling it
            assert(false && "You tried to call a non existant method");
            return nullptr;
        }

        //----------------- AUTOMATIC -----------------// // These events are called automatically

        // Called once after all components have been added
        virtual void onCreate(entt::registry& registry, entt::entity me) {}

        // Called once before the entitiy is destroyed
        virtual void onDestroy(entt::registry& registry, entt::entity me) {}

        // Called when this entity collides with another entity - called for both entities
        virtual void onDynamicCollision(entt::registry& registry, entt::entity me, entt::entity other) {}

        // Called when this entity collides with a static collision object - walls...
        virtual void onStaticCollision(entt::registry& registry, entt::entity me) {}

        // Called at the beginning of each tick - only called on entities in update range
        virtual void onTick(entt::registry& registry, entt::entity me) {}

        // Called once at the beginning of each tick IF keystate changed - press or release
        virtual void onKeyEvent(entt::registry& registry, entt::entity me) {}

        // Called once at the beginning of each tick IF mouse state changed - includes mouse movement
        virtual void onMouseEvent(entt::registry& registry, entt::entity me) {}

        //-----------------USER -----------------// // These events have to be called by the user
        // Examples:

        // virtual void onMove(entt::registry& registry, entt::entity me, float newX, float newY) {}

        // virtual void onDeath(entt::registry& registry, entt::entity me, entt::entity killedBy) {}

        // virtual void onInteract(entt::registry& registry, entt::entity me, entt::entity target) {}

        // virtual void onItemPickup(entt::registry& registry, entt::entity me, Item& item) {}

        // virtual void onLevelUp(entt::registry& registry, entt::entity me) {}

        // ... feel free to add more!
    };

    // Sets a c++ script for this entity type
    // Subclass the EntityScript class and pass a new Instance()
    void SetScript(EntityID entity, EntityScript* script);

    // Retrieves the script for the entity type
    // Failure: returns nullptr
    EntityScript* GetScript(EntityID entity);

    // Calls the given event function on given entity
    // IMPORTANT: 'arguments' are only parameters after the registry and self id - they are passed implicitly
    // Example:    InvokeEvent<onKeyEvent>(self);
    //             InvokeEvent<onDynamicCollision>(self, other);
    template <EventType event, class... Args>
    void InvokeEvent(entt::entity entity, Args... arguments)
    {
        const auto& pos = REGISTRY.get<PositionC>(entity); // Every entity has position
        EntityScript* script = GetScript(pos.type);
        if (script == nullptr) [[unlikely]]
            return;
        auto func = script->getEventFunction<event, entt::registry&, entt::entity, Args...>();
        CALL_MEMBER_FN(*script, func)(REGISTRY, entity, std::forward<Args>(arguments)...);
    }

    // Same as 'InvokeEvent' but avoids the type lookup - very fast!
    template <EventType event, class... Args>
    void InvokeEventDirect(EntityScript& script, entt::entity entity, Args... arguments)
    {
        auto func = script.getEventFunction<event, entt::registry&, entt::entity, Args...>();
        CALL_MEMBER_FN(script, func)(REGISTRY, entity, std::forward<Args>(arguments)...);
    }

} // namespace magique
#endif //MAGIQUE_INTERNAL_SCRIPTING_H