#ifndef REGISTRY_H
#define REGISTRY_H

#include <functional>
#include <magique/ecs/Components.h>
#include <entt/entity/registry.hpp>

//-------------------------------
// ECS Module
//-------------------------------
// ................................................................................
// Note: All entities have the PositionC auto assigned per default!
// ................................................................................

enum EntityID : uint16_t; // A unique type identifier handled by the user to disinguish different types of game objects

namespace magique
{
    // Returns the magique registry
    entt::registry& GetRegistry();

    //-------------- REGISTER --------------//

    using CreateFunc = std::function<void(entt::registry&, entt::entity)>;
    // Registers an entity with a corresponding create function - replaces the existing function if present
    // Failure: Returns false
    bool RegisterEntity(EntityID type, const CreateFunc& createFunc);

    // Unregisters an entity
    // Failure: Returns false
    bool UnRegisterEntity(EntityID type);

    //----------------- INTERACTION -----------------//

    // Retrieves the specified component from the public registry
    template <typename T>
    T& GetComponent(entt::entity e);

    // Uses emplace_back to add the component to the given entity
    // Args are the constructor arguments (if any)
    // IMPORTANT: Args HAVE to match type EXACTLY with the constructor or member variables (without constructor)
    template <typename Component, typename... Args>
    void GiveComponent(entt::entity e, Args... args);

    // Returns true if the given entity exist in the registry
    bool IsEntityExisting(entt::entity e);

    //-------------- LIFE CYCLE --------------//

    // Creates a new entity by calling the registered function for that type
    // Note: All entities have the PositionC auto assigned per default!
    // Failure: Returns entt::null
    entt::entity CreateEntity(EntityID type, float x, float y, MapID map);

    // Creates a new entity with the given id
    // Note: Should only be called in a networking context with a valid id
    entt::entity CreateEntityNetwork(uint32_t id, EntityID type, float x, float y, MapID map);

    // Tries to destroy the entity and all its components
    // Failure: Returns false if entity is invalid or doesnt exist
    bool DestroyEntity(entt::entity e);

    //--------------Creating--------------//

    // Makes the entity collidable with others - Shape: RECT
    // Pass the width and height of the rectangle
    CollisionC& GiveCollisionRect(entt::entity e, float width, float height, int anchorX = 0, int anchorY = 0);

    // Makes the entity collidable with others - Shape: CIRCLE (vertical)
    // Pass the height and the radius of the capsule
    CollisionC& GiveCollisionCircle(entt::entity e, float radius, int anchorX = 0, int anchorY = 0);

    // Makes the entity collidable with others - Shape: CAPSULE (vertical)
    // Pass the height and the radius of the capsule
    CollisionC& GiveCollisionCapsule(entt::entity e, float height, float radius, int anchorX = 0, int anchorY = 0);

    // Makes the entity collidable with others - Shape: TRIANGLE
    // Pass the offsets for the two remaining points in counter clockwise order - first one is (pos.x, pos.y)
    CollisionC& GiveCollisionTri(entt::entity e, float x, float y, float x2, float y2, int anchorX = 0, int anchorY = 0);

    // Makes the entitiy emit light according to the current lighting model
    EmitterC& GiveEmitter(entt::entity entity, Color color, int intensity = 100, LightStyle style = POINT_LIGHT_SOFT);

    // Makes the entitiy occlude light and throw shadows according to the current lighting model
    OccluderC& GiveOccluder(entt::entity entity, int width, int height, Shape shape = Shape::RECT);

    // Adds the camera component
    // Camera will automatically reflect the entity state (update)
    void GiveCamera(entt::entity entity);

    // Adds components such that the given entity is an actor
    void GiveActor(entt::entity entity);

    // Makes the entity react to its script
    // IMPORTANT: Entity type needs to have a script set! Use SetScript(type,new MyScript());
    void GiveScript(entt::entity entity);

} // namespace magique


//----------------- IMPLEMENTATION -----------------//

namespace magique::internal
{
    inline entt::registry REGISTRY; // The used registry - lives user side cause of big includes
}
inline entt::registry& magique::GetRegistry() { return internal::REGISTRY; }
template <typename T>
T& magique::GetComponent(const entt::entity e)
{
    return internal::REGISTRY.get<T>(e);
}
template <class Component, typename... Args>
void magique::GiveComponent(entt::entity e, Args... args)
{
    internal::REGISTRY.emplace<Component>(e, args...);
}

#endif // REGISTRY_H