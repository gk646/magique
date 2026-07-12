// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_INTERNAL_SCRIPTING_H
#define MAGIQUE_INTERNAL_SCRIPTING_H

#include <magique/ecs/ECS.h>

//===============================================
// Internal Scripting Module
//===============================================
// ................................................................................
// This module allows to create custom scripts for all entities of a type by subclassing EntityScript.
// This means it's a global instance for all entities of that type and MUST not contain any state.
// Different behavior is managed by a different component state of each entity (rather than state inside the script)
//
// Note: All entities are scripted per default - you need to explicitly disable this on a per-entity basis
// Note: You need to specify your custom type if you want to invoke a method that is specific to that type:
//      - MyEntityScript::onSit() -> ScriptingGetScript<MyEntityScript>(entity).onSit(entity);
// To create behavior hierarchies subclass EntityScript and create new functions or defaults
//      => See examples/headers/Scripting.h
// ................................................................................

namespace magique
{
    // Sets a script for this entity type (or multiple)
    // Subclass the EntityScript class and pass a new Instance()
    void ScriptingSetScript(EntityType type, EntityScript* script);
    void ScriptingSetScript(std::initializer_list<EntityType> types, EntityScript* script);

    // Retrieves the script for the entity type
    // Failure: if no script is registered for the given type returns nullptr
    // Note: By passing your custom derived class you can call new methods
    template <class Script = EntityScript>
    Script* ScriptingGetScript(Entity entity);

    // Sets the scripted status for the given entity - if set no automatic script methods will be called for this entity
    void ScriptingSetScripted(Entity entity, bool val);

    // Returns true if the given entity receives script updates
    bool ScriptingGetIsScripted(Entity entity);

    struct EntityScript
    {
        virtual ~EntityScript() = default;

        //================= AUTOMATIC =================// // These events are called automatically

        // Called once after all components have been added
        virtual void onCreate(Entity self) {}

        // Called once before the entity is destroyed
        virtual void onDestroy(Entity self) {}

        // Called once at the beginning of each tick
        //      - updated: true if this entity is in update range of any actor (e.g. it's loaded)
        virtual void onUpdate(Entity self, bool updated) {}

        // Called each time this entity collides with another entity - called for both entities
        virtual void onDynamicCollision(Entity self, Entity other, CollisionInfo& collision)
        {
            AccumulateCollision(collision); // Treats the other shape as solid per default
        }

        // Called each time this entity collides with a static collision object
        virtual void onStaticCollision(Entity self, ColliderInfo collider, CollisionInfo& collision)
        {
            AccumulateCollision(collision); // Treats the other shape as solid per default
        }

        //================= UTIL =================//

        // Adds the given info on top the existing info for this entity - will be applied after all collisions are resolved
        // Note: This essentially makes the other shape 'solid' preventing you from entering it!
        static void AccumulateCollision(CollisionInfo& collision);
    };

} // namespace magique


//================= IMPLEMENTATION =================//


namespace magique
{
    namespace internal
    {
        EntityScript* GetScriptInternal(Entity entity);
    }

    template <class Script>
    Script* ScriptingGetScript(Entity entity)
    {
        return static_cast<Script*>(internal::GetScriptInternal(entity));
    }
} // namespace magique
#endif // MAGIQUE_INTERNAL_SCRIPTING_H
