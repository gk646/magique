// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ECS_H
#define MAGIQUE_ECS_H

#include <entt/entity/registry.hpp>
#include <magique/ecs/Components.h>
#include <magique/core/Engine.h>

//===============================
// ECS Module
//===============================
// ................................................................................
// This modules serves as an abstraction over the native enTT interface.
// For more info see its documentation: https://github.com/skypjack/entt/wiki
// Note: All entities have the PositionC auto assigned per default!
// Note: Creates entities are generally only "part of the system" in the next tick!
//       Only the start of the tick, all entities are checked and marked
//       => So adding entities mid-tick will not get them collision check this tick
// ................................................................................

enum class EntityType : uint16_t; // A unique identifier handled by the user for different game objects

namespace magique
{
    using EntityCallback = std::function<void(entt::entity entity)>;

    // Returns the magique registry
    entt::registry& EntityGetRegistry();

    //============== ENTITIY ==============//

    using CreateFunc = std::function<void(entt::entity entity, EntityType type)>;

    // Registers an entity (or multiple) with the given create function - replaces the existing function if present
    // Failure: Returns false
    bool EntityRegister(EntityType type, const CreateFunc& createFunc);
    bool EntityRegister(std::initializer_list<EntityType> types, const CreateFunc& createFunc);

    // Unregisters an entity
    // Failure: Returns false
    bool EntityUnregister(EntityType type);

    // Creates a new entity by calling the registered function for that type
    // Note: All entities have the PositionC auto assigned per default!
    //      - withFunc: if true looks for and requires EntityRegister to be called first with a creation function
    // Failure: Returns entt::null
    entt::entity EntityCreate(EntityType type, Point pos, MapID map, float rotation = 0, bool withFunc = true);

    // Tries to create a new entity with the given id - will FAIL if this id is already taken
    // Note: Should only be called in a networking context with a valid id (when receiving entity info as a client)
    // Note: You shouldn't use  this information - but entity ids start with 0 and go up until UINT32_MAX
    entt::entity EntityCreateEx(entt::entity id, EntityType type, Point pos, MapID map, float rotation, bool withFunc);

    // Sets a function that is called each time AFTER the entity is created or BEFORE the entity is destroyed
    void EntitySetDestroyCallback(const EntityCallback& callback);
    void EntitySetCreateCallback(const EntityCallback& callback);

    // Returns true if the given entity exist in the registry
    bool EntityExists(entt::entity entity);

    // Returns true and executes the given function if the entity exists
    template <typename Func>
    bool EntityIfExists(entt::entity entity, const Func& func);

    // Returns true if the given entity has ALL the specified component types
    template <typename... Args>
    bool EntityHasComponents(entt::entity entity);

    // Returns true if the given entity is an actor - has the actor component
    bool EntityIsActor(entt::entity entity);

    // Returns the first entity with the given type
    // Failure: Returns entt::null if none with that type could be found
    entt::entity EntityFirstOf(EntityType type);

    // Immediately tries to destroy this entity
    // Note: It's up to the user to make sure invalid entities are not accessed (destroying in event functions...)
    // Failure: Returns false if entity is invalid or doesn't exist
    bool EntityDestroy(entt::entity entity);

    // Immediately destroys all entities that have the given type - pass an empty list to destroy all types
    void EntityDestroy(const std::initializer_list<EntityType>& ids);

    // Iterates all enemies and destroys them if the function returns true
    void EntityDestroy(const std::function<bool(entt::entity)>& func);

    //================= COMPONENTS =================//

    // Retrieves the specified component from the global registry
    // Note: When using views to iterate over entities it's faster to access components over the view
    template <typename T>
    T& ComponentGet(entt::entity entity = EngineGetPlayer());

    // Returns one or more of the specified components from the entity
    template <typename... T>
    auto ComponentsGet(entt::entity entity = EngineGetPlayer());

    // Tries to retrieve the specified component from the global registry
    // Note: When using views to iterate over entities it's faster to access components over the view
    // Failure: Returns nullptr if the component is not present on the given entity
    template <typename T>
    T* ComponentTryGet(entt::entity entity);

    // Tries to remove the specified components from the given entities
    // Note: If CollisionC is removed, immediately removes the entity from internal collision caches
    template <typename... Args>
    void ComponentRemove(entt::entity entity);

    // Returns a view that contains all entities with (all) given components
    template <typename... Args>
    auto ComponentGetView();

    // Uses emplace_back to add the component to the given entity - Args are the constructor arguments (if any)
    // Note: If the entity already has the component will crash
    // IMPORTANT: Args HAVE to match type EXACTLY with the constructor or member variables (without constructor)
    template <typename Component, typename... Args>
    Component& ComponentGive(entt::entity entity, Args... args);

    // Same as ComponentGive but if the entity already has the component returns the existing one
    template <typename Component, typename... Args>
    Component& ComponentTryGive(entt::entity entity, Args... args);

    // Makes the entity collidable with others - Shape: RECT
    // x & y of the rect is offset from the position - anchor is set to the mid-point unless specified (relative to offset)
    CollisionC& ComponentGiveCollisionRect(entt::entity entity, Rect rect, Point anchor = {-1});

    // Makes the entity collidable with others - Shape: CIRCLE
    // Circles always rotated around their middle point!
    CollisionC& ComponentGiveCollisionCircle(entt::entity entity, float radius);

    // Makes the entity collidable with others - Shape: TRIANGLE
    // Pass the offsets for the two remaining points in counterclockwise order - first one is (pos.x, pos.y)
    CollisionC& ComponentGiveCollisionTri(entt::entity entity, Point p2, Point p3, Point anchor = {});

    // Adds the camera component
    void ComponentGiveCamera(entt::entity entity);

    // Adds components such that the given entity is an actor
    void ComponentGiveActor(entt::entity entity);

} // namespace magique


//================= IMPLEMENTATION =================//

namespace magique
{
    namespace internal
    {
        inline entt::registry REGISTRY{};                                     // The used registry
        inline auto POSITION_GROUP = REGISTRY.group<PositionC, CollisionC>(); // Pos + Collision group

        void OnRemoveCollisionC(entt::entity e);

    } // namespace internal
    inline entt::registry& EntityGetRegistry() { return internal::REGISTRY; }

    template <typename T>
    T& ComponentGet(const entt::entity entity)
    {
        static_assert(sizeof(T) != 0 && "Trying to get empty component - those are not instantiated by EnTT");
        MAGIQUE_ASSERT(EntityExists(entity), "Entity does not exist");
        MAGIQUE_ASSERT(EntityHasComponents<T>(entity), "Specified component does not exist on this entity!");
        if constexpr (std::is_same_v<T, PositionC> || std::is_same_v<T, CollisionC>)
        {
            return internal::POSITION_GROUP.get<T>(entity);
        }
        else
        {
            return internal::REGISTRY.get<T>(entity);
        }
    }

    template <typename... T>
    auto ComponentsGet(entt::entity entity)
    {
        MAGIQUE_ASSERT(EntityExists(entity), "Entity does not exist");
        return internal::REGISTRY.get<T...>(entity);
    }

    template <typename T>
    T* ComponentTryGet(const entt::entity entity)
    {
        static_assert(sizeof(T) > 0 && "Trying to get empty component - those are not instantiated by EnTT");
        return internal::REGISTRY.try_get<T>(entity);
    }

    template <class Component, typename... Args>
    Component& ComponentGive(entt::entity entity, Args... args)
    {
        return internal::REGISTRY.emplace<Component>(entity, args...);
    }

    template <typename Component, typename... Args>
    Component& ComponentTryGive(entt::entity entity, Args... args)
    {
        return internal::REGISTRY.get_or_emplace<Component>(entity, args...);
    }

    template <typename T, typename... Args>
    struct contains : std::disjunction<std::is_same<T, Args>...>
    {
    };

    template <typename Func>
    bool EntityIfExists(entt::entity entity, const Func& func)
    {
        if (EntityExists(entity))
        {
            func(entity);
            return true;
        }
        return false;
    }

    template <typename... Args>
    bool EntityHasComponents(const entt::entity entity)
    {
        return internal::REGISTRY.all_of<Args...>(entity);
    }

    template <typename... Args>
    void ComponentRemove(entt::entity entity)
    {
        internal::REGISTRY.remove<Args...>(entity);
        if constexpr (contains<CollisionC, Args...>::value)
        {
            internal::OnRemoveCollisionC(entity);
        }
    }

    template <typename... Args>
    auto ComponentGetView()
    {
        return internal::REGISTRY.view<Args...>();
    }

} // namespace magique

#endif // MAGIQUE_ECS_H
