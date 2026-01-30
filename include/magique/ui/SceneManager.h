// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_SCENE_MANAGER_H
#define MAGIQUE_SCENE_MANAGER_H

#include <vector>
#include <magique/internal/InternalTypes.h>

//===============================================
// Scene Manager
//===============================================
// .....................................................................
// A scene allows you to store any UIObject's and draw them when needed. Objects can also be shared across scenes.
// .....................................................................

namespace magique
{
    // Returns the global instance of the scene manager
    SceneManager& GetSceneManager();

    struct Scene final
    {
        // Draws all objects such that they appear in the order they were added (added first => last drawn => appears on top)
        void draw();

        // Adds an object
        // Returns: The given objects
        template <typename T>
        T& addObject(T* obj, const char* name = nullptr)
        {
            static_assert(std::is_base_of_v<UIObject, T>, "T must be a subclass of UIObject");
            return *static_cast<T*>(addObjImpl(obj, name));
        }

        // Returns: The given objects casted to the given type
        template <typename T = UIObject>
        T* getObject(const char* name) const
        {
            return static_cast<T*>(getObjectImpl(name));
        }

        // Returns true if the object was removed
        bool removeObject(UIObject* obj);


        // Returns all objects
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

#endif //MAGIQUE_SCENE_MANAGER_H
