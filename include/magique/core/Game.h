#ifndef MAGIQUE_GAME_H
#define MAGIQUE_GAME_H

#pragma warning(push)
#pragma warning(disable : 4100) // unreferenced formal parameter

#include <magique/fwd.hpp>

//-----------------------------------------------
// Game module
//-----------------------------------------------
// ................................................................................
// Core game class you should subclass. You then override and implement the methods which are called automatically.
// All raylib timing and input functions work just as normal and have the same effect (e.g. SetTargetFPS(),...)
// Note: The asset image and game config are loaded with their default names if not specified.
//       To get a asset image you have to call assets/AssetPacker.h::CompileAssetImage()! (see module for more info)
//
// You should create your game and call run in the main function:
//      MyGameClass game{};
//      return game.run();
//
// Note: All functions are called on the main thread!
// Making the engine usable without the game class is a low priority task for the future.
// ................................................................................

namespace magique
{
    struct Game
    {
        explicit Game(const char* name = "MyGame");
        virtual ~Game();

        //-----------------LIFE CYCLE-----------------//

        // Called once on startup - register your loaders here
        virtual void onStartup(AssetLoader& loader, GameConfig& config) {}

        // Called once when the game closes
        virtual void onShutDown() {}

        // Called when the window close button is pressed
        // IMPORTANT: When overridden, shutDown() has to be called manually to stop the game!
        virtual void onCloseEvent() { shutDown(); }

        // Stops the game
        void shutDown();

        //----------------- CORE -----------------//

        // Called each update tick BEFORE drawGame()
        // Default: called 60 times per second (constant)
        virtual void updateGame(GameState gameState) {}

        // Called each render tick - passed the current gamestate and camera
        // Default: called 90 times per second (changable: SetTargetFPS())
        virtual void drawGame(GameState gameState, Camera2D& camera2D) {}

        // Called each render tick after drawGame()
        virtual void drawUI(GameState gameState) {}

        // Call this to start the game - should be call in the main method: return game.run();
        // Tries to load an asset image from the default path - assets will be empty if none exists!
        // Tries to load the game config from the default path - will be created if none exists!
        // Note: The encryption key is applied to both assets and config - make sure they match
        int run(const char* assetPath = "data.bin", const char* configPath = "Config.cfg", uint64_t encryptionKey = 0);

        //----------------- VARIABLES -----------------//

        [[nodiscard]] bool getIsRunning() const;
        [[nodiscard]] bool getIsLoading() const;
        [[nodiscard]] const char* getName() const;

    private:
        bool isRunning = false;
        bool isLoading = false;
        const char* gameName;
        friend void HandleLoadingScreen(Game& game);
    };

} // namespace magique

#pragma warning(pop)

#endif // MAGIQUE_GAME_H