// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_SCENE_MANAGER_H
#define MAGIQUE_SCENE_MANAGER_H

#include <vector>
#include <magique/fwd.hpp>

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
        // Draws all objects in the reverse order they were added (first one added = last drawn = drawn on top)
        void draw();

        // Adds or removes an object
        void addObject(UIObject* obj);

        // Returns true if the object was removed
        bool removeObject(UIObject* obj);

        // Returns all objects
        const std::vector<UIObject*>& getObjects();

    private:
        std::vector<UIObject*> objects;
    };

    struct SceneManager final
    {
        // Returns the scene identified with the given name
        // Note: If noone exists yet a new one is created
        Scene& getScene(const char* name);

        // Returns a scene identified with the given gamestate
        // Note: Useful for when you want to have a scene for each gamestate
        Scene& getScene(GameState state);
    };
} // namespace magique

#endif //MAGIQUE_SCENE_MANAGER_H