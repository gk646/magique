#ifndef REGISTRY_H
#define REGISTRY_H

#include <functional>
#include <entt/entity/registry.hpp>

#include "BaseComponents.h" // The base components

//-------------------------------
// ECS Module
//-------------------------------
// .....................................................................
// All entities have the PositionC auto assigned per default
// .....................................................................

// A unique type identifier handled by the user to disinguish different types of game objects
// Max value is reserved for internal use e.g. INT32_MAX

enum EntityID : uint16_t; // User implemented


namespace magique
{
    inline entt::registry REGISTRY; // The used registry - lives user side cause of massive includes
    //--------------Registering--------------//

    using CreateFunc = std::function<void(entt::registry&, entt::entity)>;
    // Registers an entity with a corresponding create function - replaces the existing function if present
    // Failure: Returns false
    bool RegisterEntity(EntityID type, const CreateFunc& createFunc);

    // Unregisters an entity
    // Failure: Returns false
    bool UnRegisterEntity(EntityID type);

    //--------------LIFE CYCLE--------------//

    // Creates a new entity by calling the registered function for that type
    // Failure: Returns entt::null
    entt::entity CreateEntity(EntityID type, float x, float y, MapID map);

    // Tries to destroy the entity and all its components
    // Failure: Returns false
    bool DestroyEntity(entt::entity e);

    // auto getComponent() -> use native registry with templated access
    // GetNativeRegistry().get<PositionC>(entt::entity);

    // auto getView() -> use native registry with templated access
    // GetNativeRegistry().view<PositionC>();

    //--------------Creating--------------//

    // Adds components such that the given entity is an actor
    void GiveActor(entt::entity e);

    // Makes the entity collidable with others
    void GiveCollision(entt::entity e, Shape shape, int width, int height, int anchorX = 0, int anchorY = 0);

    // Draws entitiy as rectangles - only works in debug mode
    void GiveDebugVisuals(entt::entity e);

    // Gives it w,a,s,d controls - only works in debug mode
    void GiveDebugController(entt::entity e);

} // namespace magique::ecs

#endif // REGISTRY_H