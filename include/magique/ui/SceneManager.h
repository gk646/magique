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

        // Adds or removes an object
        // Returns: The given objects
        UIObject* addObject(UIObject* obj, const char* name = nullptr);

        // Returns true if the object was removed
        bool removeObject(UIObject* obj);

        UIObject* getObject(const char* name) const;

        // Returns all objects
        const std::vector<UIObject*>& getObjects();

    private:
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