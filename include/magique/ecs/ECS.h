#ifndef REGISTRY_H
#define REGISTRY_H

#include <functional>
#include <magique/ecs/Components.h>
#include <entt/entity/registry.hpp>

//-------------------------------
// ECS Module
//-------------------------------
// ................................................................................
// All entities have the PositionC auto assigned per default
// ................................................................................

// A unique type identifier handled by the user to disinguish different types of game objects
// Max value is reserved for internal use e.g. UINT16_MAX
enum EntityID : uint16_t; // User implemented

namespace magique
{
    // Returns the magique registry
    entt::registry& GetRegistry();

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
    // GetRegistry().get<PositionC>(entt::entity);

    // auto getView() -> use native registry with templated access
    // GetRegistry().view<PositionC>();

    //--------------Creating--------------//

    // Note: All Create__ functions return a reference to the created component (where appropriate)

    // Makes the entity collidable with others
    CollisionC& GiveCollision(entt::entity entity, Shape shape, int width, int height, int anchorX = 0,
                              int anchorY = 0);

    // Makes the entitiy emit light according to the current lighting model
    EmitterC& GiveEmitter(entt::entity entity, Color color, int intensity = 100, LightStyle style = POINT_LIGHT_SOFT);

    // Makes the entitiy occlude light and throw shadows according to the current lighting model
    OccluderC& GiveOccluder(entt::entity entity, int width, int height, Shape shape = RECT);

    // Adds the camera component
    // Camera will automatically reflect the entity state (update)
    void GiveCamera(entt::entity entity);

    // Adds components such that the given entity is an actor
    void GiveActor(entt::entity entity);

    // Makes the entity react to its script
    // IMPORTANT: Entity type needs to have a script set! Use SetScript(type,new MyScript());
    void GiveScript(entt::entity entity);

    //----------------- DEBUG -----------------//

    // Draws entitiy as rectangles - only works in debug mode
    void GiveDebugVisuals(entt::entity entity);

    // Gives it w,a,s,d controls - only works in debug mode
    void GiveDebugController(entt::entity entity);

    //----------------- IMPLEMENTATION -----------------//

    inline entt::registry REGISTRY; // The used registry - lives user side cause of massive includes

    inline entt::registry& GetRegistry() { return REGISTRY; }

} // namespace magique


#endif // REGISTRY_H