#ifndef REGISTRY_H
#define REGISTRY_H

#include <functional>
#include <entt/entity/fwd.hpp>

#include "BaseComponents.h" // The base components

//-------------------------------
// ECS Module
//-------------------------------
// .....................................................................
// All entities will have the PositionC component per default
// .....................................................................

// A unique type identifier handled by the user to disinguish different types of game objects
// Max value is reserved for internal use e.g. INT32_MAX

enum class EntityType : uint16_t;

using CreateFunc = std::function<void(entt::registry&, entt::entity)>;

namespace magique::ecs
{
    //--------------Registering--------------//

    // Registers an entity with a corresponding create function - replaces the existing function if present
    // Failure: Returns false
    bool RegisterEntity(EntityType type, const CreateFunc& createFunc);

    // Unregisters an entity
    // Failure: Returns false
    bool UnRegisterEntity(EntityType type);

    //--------------LIFE CYCLE--------------//

    // Creates a new entity by calling the registered function for that type
    // Failure: Returns entt::null
    entt::entity CreateEntity(EntityType type, float x, float y, MapID map);

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

    // Returns the entt registry
    entt::registry& GetRegistry();
} // namespace magique::ecs

#endif // REGISTRY_H