#ifndef APPLICATION_H
#define APPLICATION_H

#include <entt/entity/fwd.hpp>

#include <raylib.h>
#include <magique/loading/GameLoader.h>


//-----------------------------------------------
// Game module
//-----------------------------------------------

// .....................................................................
// Core game class you should subclass
// All methods are called on the main thread unless specified otherwise.
// .....................................................................

namespace magique
{
    struct Game
    {
        Camera2D camera{};
        explicit Game(const char* name = "MyGame");
        virtual ~Game();

        //-----------------LIFE CYCLE-----------------//

        // Called on startup - register your loaders here
        virtual void onStartup(GameLoader& gl) {}

        // Called when the windows close button is pressed
        virtual void onCloseEvent() { isRunning = false; }

        // Called when the game closes
        virtual void onShutDown() {}

        // Calls the close event
        void shutDown();

        //-----------------UPDATING-----------------//

        // Called each update tick
        // Thread: Called on the update thread!
        virtual void updateGame(entt::registry& registry) {}

        //-----------------RENDERING-----------------//

        // Called each tick before rendering happens
        virtual void preRender() {}

        // Called each tick when loading
        virtual void drawLoadingScreen(float progressPercent) {}

        // Called each render tick
        virtual void drawGame(entt::registry& registry, Camera2D& camera) {}

        // Called each render tick after the drawGame call
        virtual void drawUI() {}

        // Call this to start the game
        int run(const char* assetPath = "data.bin", uint64_t encryptionKey = 0);

    private:
        bool isRunning = false;
        const char* const gameName;
    };

} // namespace magique


#endif // APPLICATION_H