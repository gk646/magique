#ifndef SCENE_H
#define SCENE_H

#include <utility>
#include <magique/ui/types/UIObject.h>

//-----------------------------------------------
// Scene
//-----------------------------------------------
// .....................................................................
// Scenes are user managed containers for UIObjects.
// Note: A UIObject can only be created in connection to a scene!
// Scenes need to be drawn manually (but are updated automatically!)
// .....................................................................

namespace magique
{
    // Registers a new scene - accepts inherited classed aswell
    // Takes ownership of the pointer - pass with "new MyScene()"
    // Failure: returns false if given name is not unique
    bool RegisterScene(UIScene* scene, const char* name);

    // Returns the scene witht the given name - optionally specify your custom type
    // Example: MyScene& myScene = GetScene<MyScene>("MyScene")
    // Failure: if name is not unique returns nullptr
    template <class T = UIScene>
    T& GetScene(const char* name);

    struct UIScene
    {
        // Adds a new UIObject to the scene
        // If needed the returned pointer can be stored inside the scene - same lifetime as the scene
        template <typename T, typename... Args>
        T* addUIObject(Args&&... args);

        // Submits this scene for deferred rendering and updates for this tick
        // Rendering happens after Game::drawUI() returns
        // Should be called every render tick if scene is visible
        void draw() const;

        //----------------- EVENTS -----------------//

        // Called BEFORE the scene is updated
        virtual void onUpdate() {}

        // Called BEFORE the scene is rendered
        virtual void onDraw() {}

        UIScene();

    private:
        int dataIdx = -1;
        void addObjetctImpl(UIObject* object) const;
    };

} // namespace magique

namespace magique
{
    UIScene* GetSceneImpl(const char* name);
    template <typename T>
    T& GetScene(const char* name)
    {
        return *static_cast<T*>(GetSceneImpl(name));
    }
    template <typename T, typename... Args>
    T* UIScene::addUIObject(Args&&... args)
    {
        auto* object = new T(std::forward<Args>(args)...);
        addObjetctImpl(object);
        return *object;
    }
} // namespace magique
#endif //SCENE_H