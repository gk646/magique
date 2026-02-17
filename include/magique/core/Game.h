// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_GAME_H
#define MAGIQUE_GAME_H

#include <magique/ecs/GameSystem.h>

//===============================================
// Game module
//===============================================
// ................................................................................
// Core game class you should subclass. You then override and implement the methods which are called automatically.
// All raylib timing and input functions work just as normal and have the same effect (e.g. SetTargetFPS(),...)
// Note: The asset pack and game config are loaded with their default names if not specified.
//       To get an asset pack you have to call assets/AssetPacker.h::AssetPackCompile()! (see module for more info)
//
// You should create your game and call run in the main function:
//      MyGameClass game{};
//      return game.run();
//
// Note: All functions are called on the main thread!
// Note: Inputs are polled before each UpdateTick only! So polling for keyboard input (besides IsDown() or IsUp)
//        is NOT accurate in drawGame()!
// Making the engine usable without the game class is a low priority task for the future.
// ................................................................................

namespace magique
{

    // Returns the global game instance
    // IMPORTANT: only valid after the game constructor finished (so before game.run(), ... so pretty early)
    Game& GameGetInstance();

    struct Game
    {
        // Constructs a game instance - sets the window title to the passed name
        // Note: This MUST only be called once per program
        explicit Game(const char* name = "MyGame", const char* version = "0.0.0");
        virtual ~Game();

        // Call this to start the game - should be call in the main method: return game.run();
        // Tries to load an asset pack from the default path - assets will be empty if none exists!
        // Tries to load the game config from the default path - will be created if none exists!
        // Note: The encryption key is applied to both assets and config - make sure they match
        int run(const char* assetPath = "data.bin", const char* configPath = "Config.cfg", uint64_t encryptionKey = 0);

        // Stops the game gracefully - calls onShutDown()
        void shutDown();

        //================= LIFE CYCLE =================//

        // Called once on startup - register your loaders here
        // This is where you access your assets that are loaded from the asset pack
        // Note: The registered tasks are executed after this method returns - to access results use onLoadingFinished()
        virtual void onStartup(AssetLoader& loader) {}

        // Called once on startup when all registered task have been loaded
        virtual void onLoadingFinished() {}

        // Called when the window close button is pressed
        // IMPORTANT: When overridden, shutDown() has to be called manually to stop the game!
        virtual void onCloseEvent() { shutDown(); }

        // Called once before the game closes
        virtual void onShutDown() { GameSystemPrintStats(); }

        //================= CORE =================//

        // Called each render tick - should draw the world here
        // Default: called 100 times per second - changed by SetTargetFPS()
        virtual void onDrawGame(GameState gameState, Camera2D& camera2D) { GameSystem::CallDrawGame(); }

        // Called each render tick after drawGame() - should draw the user interface here
        // EndDrawing() will be called after this method internally
        virtual void onDrawUI(GameState gameState) {}

        // Called each update tick BEFORE drawGame()
        // Default: called 60 times per second (constant)
        virtual void onUpdateGame(GameState gameState) { GameSystem::CallUpdateGame(); }

        // Called after the internal update tick (collision, ui, sound)
        // Note: Useful for sending network updates as this it the final state for this tick
        virtual void onUpdateEnd(GameState gameState) { GameSystem::CallUpdateEnd(); }

        //================= VARIABLES =================//

        [[nodiscard]] bool getIsRunning() const;
        [[nodiscard]] bool getIsLoading() const;
        [[nodiscard]] const char* getName() const;
        [[nodiscard]] const char* getVersion() const;

    private:
        bool isRunning = false;
        bool isLoading = false;
        const char* gameName;
        const char* version;
        friend void HandleLoadingScreen(Game& game);
    };

} // namespace magique

#endif // MAGIQUE_GAME_H
