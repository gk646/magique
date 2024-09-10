#ifndef MAGIQUE_ECS_H
#define MAGIQUE_ECS_H

#include <entt/entity/registry.hpp>
#include <magique/ecs/Components.h>

//-------------------------------
// ECS Module
//-------------------------------
// ................................................................................
// All entities have the PositionC auto assigned per default!
// ................................................................................

enum EntityType : uint16_t; // A unique type identifier handled by the user to distinguish different types of game objects

namespace magique
{
    // Returns the magique registry
    entt::registry& GetRegistry();

    //-------------- REGISTER --------------//

    using CreateFunc = std::function<void(entt::entity entity, EntityType type)>;
    // Registers an entity with the given create function - replaces the existing function if present
    // Failure: Returns false
    bool RegisterEntity(EntityType type, const CreateFunc& createFunc);

    // Unregisters an entity
    // Failure: Returns false
    bool UnRegisterEntity(EntityType type);

    //----------------- INTERACTION -----------------//

    // Retrieves the specified component from the public registry
    template <typename T>
    T& GetComponent(entt::entity entity);

    // Uses emplace_back to add the component to the given entity
    // Args are the constructor arguments (if any)
    // IMPORTANT: Args HAVE to match type EXACTLY with the constructor or member variables (without constructor)
    template <typename Component, typename... Args>
    void GiveComponent(entt::entity entity, Args... args);

    // Returns true if the given entity exist in the registry
    bool EntityExists(entt::entity entity);

    // Returns true if the given entity has ALL the specified component types
    template <typename... Args>
    bool EntityHasComponents(entt::entity entity);

    // Returns true if the given entity is an actor - has the actor component
    bool EntityIsActor(entt::entity entity);

    //-------------- LIFE CYCLE --------------//

    // Creates a new entity by calling the registered function for that type
    // Note: All entities have the PositionC auto assigned per default!
    // Failure: Returns entt::null
    entt::entity CreateEntity(EntityType type, float x, float y, MapID map);

    // Creates a new entity with the given id
    // Note: Should only be called in a networking context with a valid id
    entt::entity CreateEntityNetwork(uint32_t id, EntityType type, float x, float y, MapID map);

    // Immediately tries destroys the entity
    // Note: it's up to the user to make sure invalid entities are not accessed (destroying in event functions...)
    // Failure: Returns false if entity is invalid or doesn't exist
    bool DestroyEntity(entt::entity entity);

    // Immediately destroys all entities that have the given type - pass an empty list to destroy all types
    void DestroyEntities(const std::initializer_list<EntityType>& ids);

    //--------------Creating--------------//

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

    // Makes the entity react to its script
    // IMPORTANT: Entity type needs to have a script set! Use SetScript(type,new MyScript());
    void GiveScript(entt::entity entity);

} // namespace magique


//----------------- IMPLEMENTATION -----------------//

namespace magique
{
    namespace internal
    {
        inline entt::registry REGISTRY;                                       // The used registry
        inline auto POSITION_GROUP = REGISTRY.group<PositionC, CollisionC>(); // Pos + Collision group

    } // namespace internal
    inline entt::registry& GetRegistry() { return internal::REGISTRY; }
    template <typename T>
    T& GetComponent(const entt::entity entity)
    {
        if constexpr (std::is_same_v<T, PositionC> || std::is_same_v<T, CollisionC>)
        {
            return internal::POSITION_GROUP.get<T>(entity);
        }
        else
        {
            return internal::REGISTRY.get<T>(entity);
        }
    }
    template <class Component, typename... Args>
    void GiveComponent(entt::entity entity, Args... args)
    {
        internal::REGISTRY.emplace<Component>(entity, args...);
    }
    template <typename... Args>
    bool EntityHasComponents(const entt::entity entity)
    {
        return internal::REGISTRY.all_of<Args...>(entity);
    }
} // namespace magique

#endif // MAGIQUE_ECS_H