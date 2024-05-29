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

    // Registers an entity
    // Failure: Returns false
    bool RegisterEntity(EntityType type, const CreateFunc& createFunc);

    // Unregisters an entity - replaces the existing function if present
    // Failure: Returns false
    bool UnRegisterEntity(EntityType type);

    //--------------LIFE CYCLE--------------//

    // Creates a new entity by calling the registered function for that type
    // Failure: Returns entt::null
    entt::entity CreateEntity(EntityType type);

    // Tries to destroy the entity and all its components
    // Failure: Returns false
    bool DestroyEntity(entt::entity e);

    // auto getComponent() -> use native registry with templated access
    // GetNativeRegistry().get<PositionC>(entt::entity);

    // auto getView() -> use native registry with templated access
    // GetNativeRegistry().view<PositionC>();

    //--------------Creating--------------//

    // Adds components such that the given entity is an actor
    void MakeActor(entt::entity e);

    // Makes the entity collidable with others
    void MakeCollision(entt::entity e, Shape shape, int anchorX = 0, int anchorY = 0);

    // Returns the entt registry
    entt::registry& GetRegistry();
} // namespace magique::ecs

#endif // REGISTRY_H