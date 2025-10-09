// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ECS_H
#define MAGIQUE_ECS_H

#include <entt/entity/registry.hpp>
#include <magique/ecs/Components.h>
#include <magique/internal/Macros.h>

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

enum EntityType : uint16_t; // A unique type identifier handled by the user to distinguish different types of game objects

namespace magique
{
    // Returns the magique registry
    entt::registry& GetRegistry();

    //============== REGISTER ==============//

    using CreateFunc = std::function<void(entt::entity entity, EntityType type)>;
    // Registers an entity with the given create function - replaces the existing function if present
    // Failure: Returns false
    bool RegisterEntity(EntityType type, const CreateFunc& createFunc);

    // Creates a new entity by calling the registered function for that type
    // Note: All entities have the PositionC auto assigned per default!
    //      - withFunc: if true looks for and requires RegisterEntity to be called first with a creation function
    // Failure: Returns entt::null
    entt::entity CreateEntity(EntityType type, float x, float y, MapID map, int rotation = 0, bool withFunc = true);

    // Tries to create a new entity with the given id - will FAIL if this id is already taken
    // Note: Should only be called in a networking context with a valid id (when receiving entity info as a client)
    // Note: You shouldnt use  this information - but entity ids start with 0 and go up until UINT32_MAX
    entt::entity CreateEntityEx(entt::entity id, EntityType type, float x, float y, MapID map, int rot, bool withFunc);

    // Unregisters an entity
    // Failure: Returns false
    bool UnRegisterEntity(EntityType type);

    //================= INTERACTION =================//

    // Retrieves the specified component from the global registry
    // Note: When using views to iterate over entities it's faster to access components over the view
    template <typename T>
    T& GetComponent(entt::entity entity);

    // Returns one or more of the specified components from the entity
    template <typename... T>
    auto GetComponents(entt::entity entity);

    // Tries to retrieve the specified component from the global registry
    // Note: When using views to iterate over entities it's faster to access components over the view
    // Failure: Returns nullptr if the component is not present on the given entity
    template <typename T>
    T* TryGetComponent(entt::entity entity);

    // Uses emplace_back to add the component to the given entity
    // Args are the constructor arguments (if any)
    // IMPORTANT: Args HAVE to match type EXACTLY with the constructor or member variables (without constructor)
    template <typename Component, typename... Args>
    Component& GiveComponent(entt::entity entity, Args... args);

    // Returns true if the given entity exist in the registry
    bool EntityExists(entt::entity entity);

    // Returns true if the given entity has ALL the specified component types
    template <typename... Args>
    bool EntityHasComponents(entt::entity entity);

    // Returns true if the given entity is an actor - has the actor component
    bool EntityIsActor(entt::entity entity);

    // Returns the first entity with the given type
    // Failure: Returns entt::null if none with that type could be found
    entt::entity GetEntity(EntityType type);

    // Returns a view that contains all entities with (all) given components
    template <typename... Args>
    auto GetView();

    //============== LIFE CYCLE ==============//

    // Immediately tries to destroy this entity
    // Note: It's up to the user to make sure invalid entities are not accessed (destroying in event functions...)
    // Failure: Returns false if entity is invalid or doesn't exist
    bool DestroyEntity(entt::entity entity);

    // Immediately destroys all entities that have the given type - pass an empty list to destroy all types
    void DestroyEntities(const std::initializer_list<EntityType>& ids);

    // Sets a function that is called each time a entity is destroyed
    // See core/Types.h for more info on the functino
    void SetDestroyEntityCallback(DestroyEntityCallback callback);

    //============== COMPONENTS ==============//

    // Makes the entity collidable with others - Shape: RECT
    // Pass the width and height of the rectangle
    CollisionC& GiveCollisionRect(entt::entity entity, float width, float height, int anchorX = 0, int anchorY = 0);

    // Makes the entity collidable with others - Shape: CIRCLE (vertical)
    // Pass the height and the radius of the capsule - circles always rotated around their middle point!
    CollisionC& GiveCollisionCircle(entt::entity entity, float radius);

    // Makes the entity collidable with others - Shape: CAPSULE (vertical)
    // Pass the height and the radius of the capsule - a capsule cant be rotated
    // Note: The total height must be greater than 2 * radius!
    CollisionC& GiveCollisionCapsule(entt::entity entity, float height, float radius);

    // Makes the entity collidable with others - Shape: TRIANGLE
    // Pass the offsets for the two remaining points in counterclockwise order - first one is (pos.x, pos.y)
    CollisionC& GiveCollisionTri(entt::entity entity, Point p2, Point p3, int anchorX = 0, int anchorY = 0);

    // Gives the entity access to shared animation data for that entity type
    // Note: Use the core/Animations.h module to create an EntityAnimation
    // Failure: fails fatally if there is no animation registered for that type
    AnimationC& GiveAnimation(entt::entity entity, EntityType type, AnimationState startState = {});

    // Makes the entity emit light according to the current lighting model
    EmitterC& GiveEmitter(entt::entity entity, Color color, int intensity = 100, LightStyle style = POINT_LIGHT_SOFT);

    // Makes the entity occlude light and throw shadows according to the current lighting model
    OccluderC& GiveOccluder(entt::entity entity, int width, int height, Shape shape = Shape::RECT);

    // Adds the camera component
    void GiveCamera(entt::entity entity);

    // Adds components such that the given entity is an actor
    void GiveActor(entt::entity entity);

} // namespace magique


//================= IMPLEMENTATION =================//

namespace magique
{
    namespace internal
    {
        inline entt::registry REGISTRY{};                                     // The used registry
        inline auto POSITION_GROUP = REGISTRY.group<PositionC, CollisionC>(); // Pos + Collision group

    } // namespace internal
    inline entt::registry& GetRegistry() { return internal::REGISTRY; }

    template <typename T>
    T& GetComponent(const entt::entity entity)
    {
        static_assert(sizeof(T) > 0 && "Trying to get empty component - those are not instantiated by EnTT");
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
    auto GetComponents(entt::entity entity)
    {
        MAGIQUE_ASSERT(EntityExists(entity), "Entity does not exist");
        return internal::REGISTRY.get<T...>(entity);
    }

    template <typename T>
    T* TryGetComponent(const entt::entity entity)
    {
        static_assert(sizeof(T) > 0 && "Trying to get empty component - those are not instantiated by EnTT");
        return internal::REGISTRY.try_get<T>(entity);
    }

    template <class Component, typename... Args>
    Component& GiveComponent(entt::entity entity, Args... args)
    {
        return internal::REGISTRY.emplace<Component>(entity, args...);
    }

    template <typename... Args>
    bool EntityHasComponents(const entt::entity entity)
    {
        return internal::REGISTRY.all_of<Args...>(entity);
    }

    template <typename... Args>
    auto GetView()
    {
        return internal::REGISTRY.view<Args...>();
    }

} // namespace magique

#endif // MAGIQUE_ECS_H