#ifndef MAGIQUE_GAME_H
#define MAGIQUE_GAME_H

#pragma warning(push)
#pragma warning(disable : 4100) // unreferenced formal parameter

#include <magique/fwd.hpp>
#include <entt/entity/fwd.hpp>

//-----------------------------------------------
// Game module
//-----------------------------------------------
// ................................................................................
// Core game class you should subclass
// All methods are called on the main thread unless specified otherwise.
// A note on the threading behavior:
// There is a update thread and the main thread. They almost fully run in paralell expect when some shared state is mutated
// So if you add or destoy entities they have to be synced. If you use the ECS API of magique you dont have to worry about that
// Also magique uses primitive atomics with spinlocks to avoid context switching! Generally 95% of the time no thread will wait!
// All input getters of raylib work on either thread correctly and are updated at the end of each update tick!
// ................................................................................

namespace magique
{
    struct Game
    {
        explicit Game(const char* name = "MyGame");
        virtual ~Game();

        //-----------------LIFE CYCLE-----------------//

        // Called on startup - register your loaders here
        virtual void onStartup(AssetLoader& al, GameConfig& config) {}

        // Called when the game closes
        virtual void onShutDown(GameConfig& config) {}

        // Called when the window close button is pressed
        virtual void onCloseEvent() { shutDown(); }

        // Stops the game
        void shutDown();

        //-----------------UPDATING-----------------//

        // Called each update tick
        // Thread: Called on the update thread!
        virtual void updateGame(entt::registry& registry) {}

        //-----------------RENDERING-----------------//  // In chronological call order

        // Called each tick before rendering happens
        virtual void preRender() {}

        // Called each tick when loading - skips all other draw methods
        virtual void drawLoadingScreen(UIRoot& root, float progressPercent) {}

        // Called each tick to render the world
        // This is not thread synced - dont read any logic tick data or the registry
        // This should just be pure tile drawing of the background
        virtual void drawWorld(Camera2D& camera) {}

        // Called each render tick
        // Synchronized with the logic tick - its save to access logic tick data (drawVec) or the registry
        // Note: BeginMode2D is already called
        virtual void drawGame(entt::registry& registry, Camera2D& camera) {}

        // Called each render tick after all other draw calls
        virtual void drawUI(UIRoot& root) {}

        //----------------- START -----------------//

        // Call this to start the game
        // Tries to load an asset image from the default path
        // Tries to load the game config from the default path
        int run(const char* assetPath = "data.bin", const char* configPath = "Config.cfg", uint64_t encryptionKey = 0);

        //----------------- GETTERS -----------------//

        [[nodiscard]] bool getIsRunning() const { return isRunning; }
        [[nodiscard]] bool getIsLoading() const { return isLoading; }
        [[nodiscard]] const char* getName() const { return gameName; }

    private:
        bool isRunning = false;
        bool isLoading = false;
        const char* gameName;
        friend void HandleLoadingScreen(Game& game);
        friend void Run(Game& game);
    };

} // namespace magique

#pragma warning(pop)

#endif // MAGIQUE_GAME_H