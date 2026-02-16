// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_SCENE_MANAGER_H
#define MAGIQUE_SCENE_MANAGER_H

#include <vector>
#include <magique/internal/InternalTypes.h>

//===============================================
// Scene
//===============================================
// .....................................................................
// A scene allows you to organize any UIObject's into reusable groups.
// Objects can also be shared across scenes. A scene can then be drawn to draw all contained UIObject's.
//
// Note: Scenes can be created for any given name or gamestate
// .....................................................................

namespace magique
{
    // Returns the global instance of the scene manager
    SceneManager& SceneGetManager();

    struct Scene final
    {
        // Draws all objects such that they appear in the order they were added.
        // Added first => last drawn => appears on top
        void draw() const;

        // Adds an object
        // Returns: The given objects
        template <typename T>
        T& addObject(T* obj, const char* name = nullptr);

        // Returns true if the object was removed
        bool removeObject(UIObject* obj);

        // Returns: The given object cast to the given type
        template <typename T = UIObject>
        T* getObject(const char* name) const;

        // Returns: all contained objects
        const std::vector<UIObject*>& getObjects();

    private:
        UIObject* addObjImpl(UIObject* obj, const char* name);
        UIObject* getObjectImpl(const char* name) const;

        std::vector<UIObject*> objects;
        std::vector<internal::SceneManagerMapping> mappings;
    };

    struct SceneManager final
    {
        // Returns the scene identified with the given name
        // Note: If none exists yet a new one is created
        Scene& getScene(const char* name);

        // Returns a scene identified with the given gamestate
        // Note: Useful for when you want to have a scene for each gamestate
        // Note: If none exists yet a new one is created
        Scene& getScene(GameState state);
    };
} // namespace magique


// IMPLEMENTATION


namespace magique
{

    template <typename T>
    T& Scene::addObject(T* obj, const char* name)
    {
        static_assert(std::is_base_of_v<UIObject, T>, "T must be a subclass of UIObject");
        return *static_cast<T*>(addObjImpl(obj, name));
    }

    template <typename T>
    T* Scene::getObject(const char* name) const
    {
        return static_cast<T*>(getObjectImpl(name));
    }
} // namespace magique
#endif // MAGIQUE_SCENE_MANAGER_H
