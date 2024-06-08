#ifndef APPLICATION_H
#define APPLICATION_H

#include <entt/entity/fwd.hpp>
#include <raylib/raylib.h>
#include <magique/core/GameLoader.h>

//-----------------------------------------------
// Game module
//-----------------------------------------------

// .....................................................................
// Core game class you should subclass
// All methods are called on the main thread unless specified otherwise.
// A note on the threading behavior:
// There is a update thread and the main thread. They almost fully run in paralell expect when some shared state is mutated
// So if you add or destoy entities there has to be locks. If you use the ecs API of magique you dont have to worry about that
// Also magique uses primitive atomics with spinlocks to avoid context switching! Generally 99% of the time no thread will wait!
//
// All input getters of raylib work on either thread correctly and are updated at the end of each update tick!
// .....................................................................

namespace magique
{
    struct Game
    {
        const char* const gameName;

        explicit Game(const char* name = "MyGame");
        virtual ~Game();

        //-----------------LIFE CYCLE-----------------//

        // Called on startup - register your loaders here
        virtual void onStartup(GameLoader& gl) {}

        // Called when the windows close button is pressed
        virtual void onCloseEvent() { _isRunning = false; }

        // Called when the game closes
        virtual void onShutDown() {}

        // Calls the close event
        void shutDown();

        //-----------------UPDATING-----------------//

        // Called each update tick
        // Thread: Called on the update thread!
        virtual void updateGame(entt::registry& registry) {}

        //-----------------RENDERING-----------------//  // In chronological call order

        // Called each tick before rendering happens
        virtual void preRender() {}

        // Called each tick when loading - skips all other draw methods
        virtual void drawLoadingScreen(float progressPercent) {}

        // Called each tick to render the world
        // This is not thread synced - dont read any logic tick data or the registry
        // This should just be pure tile drawing of the background
        virtual void drawWorld(Camera2D& camera) {}

        // Called each render tick
        // Synchronized with the logic tick - its save to access logic tick data (drawVec) or the registry
        // Note: BeginMode2D is already called
        virtual void drawGame(entt::registry& registry, Camera2D& camera) {}

        // Called each render tick after all other draw calls
        virtual void drawUI() {}

        //----------------- START -----------------//

        // Call this to start the game
        // Tries to load an asset image from the default path
        int run(const char* assetPath = "data.bin", uint64_t encryptionKey = 0);

        //----------------- GETTERS -----------------//

        [[nodiscard]] bool isRunning() const { return _isRunning; }
        [[nodiscard]] bool isLoading() const { return _isLoading; }

    private:
        bool _isRunning = false;
        bool _isLoading = false;
    };

} // namespace magique


#endif // APPLICATION_H