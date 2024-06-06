#ifndef MAGIQUE_EXTERNAL_SCRIPTING_H
#define MAGIQUE_EXTERNAL_SCRIPTING_H

#include <magique/fwd.hpp>
#include <magique/ecs/ECS.h>

//-----------------------------------------------
// External Scripting Module
//-----------------------------------------------
// .....................................................................
// UNFINISHED
//

namespace magique
{

    // Sets a lua script for this entity type
    void SetScriptForEntity(EntityID type, const Asset& script);

    void InvokeEvent(EventType type, entt::entity me, entt::entity target);

    bool RegisterEvent(EventType type);

    template <class Component>
    bool RegisterType(const char* name)
    {
        return RegisterGetComponent(name, [](entt::registry& reg, entt::entity e) { return reg.get<Component>(e); });
    }


} // namespace magique::ecs


#endif //MAGIQUE_EXTERNAL_SCRIPTING_H