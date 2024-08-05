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
// Core game class you should subclass. You then override and implement the methods which are then called automatically.
// All raylib timing and input functions work just as normal and have the same effect (e.g. SetTargetFPS(),...)
// Note: The asset image and game config are loaded with their default names if not specified.
//       To get assets you have to call assets/AssetPacker::CompileAssetImage()! Read its documentation for more infos.
// You should create your game and call run in the main function:
//      MyGameClass game{};
//      return game.run();
//
// Making the engine usable without the game class is a low priority task for the future.
// All functions are called on the main thread
// ................................................................................

namespace magique
{
    struct Game
    {
        explicit Game(const char* name = "MyGame");
        virtual ~Game();

        //-----------------LIFE CYCLE-----------------//

        // Called once on startup - register your loaders here
        virtual void onStartup(AssetLoader& al, GameConfig& config) {}

        // Called once when the game closes
        virtual void onShutDown(GameConfig& config) {}

        // Called when the window close button is pressed
        // Note: If overridden you have to call shutDown() manually to close the game!
        virtual void onCloseEvent() { shutDown(); }

        // Stops the game
        void shutDown();

        //-----------------UPDATING-----------------//

        // Called each update tick
        virtual void updateGame(entt::registry& registry) {}

        //-----------------RENDERING-----------------//  // In chronological call order

        // Called each tick when loading - skips all other draw methods
        virtual void drawLoadingScreen(UIRoot& root, float progressPercent) {}

        // Called each tick to render the world
        // Should be used to draw the background world
        virtual void drawWorld(Camera2D& camera) {}

        // Called each render tick
        // Note: BeginMode2D is already called - everything inside this method happens relative to the camera
        virtual void drawGame(entt::registry& registry, Camera2D& camera) {}

        // Called each render tick after all other draw calls
        virtual void drawUI(UIRoot& root) {}

        //----------------- START -----------------//

        // Call this to start the game - should be call in the main method: return game.run();
        // Tries to load an asset image from the default path - assets will be empty if none exists!
        // Tries to load the game config from the default path - will be created if none exists!
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
    };

} // namespace magique

#pragma warning(pop)

#endif // MAGIQUE_GAME_H