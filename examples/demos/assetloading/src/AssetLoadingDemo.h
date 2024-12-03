// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSET_LOADING_DEMO_H
#define MAGIQUE_ASSET_LOADING_DEMO_H

#include <string>
#include <magique/assets/AssetLoader.h>
#include <magique/assets/AssetManager.h>
#include <magique/assets/HandleRegistry.h>
#include <magique/core/Draw.h>
#include <magique/core/Game.h>
#include <raylib/raylib.h>

//===============================================
// AssetLoadingDemo
//===============================================
// ................................................................................
// IMPORTANT: checkout main.cpp to see how to compile the asset image
// magique automatically loads the image from the provided path in Game::run() when it starts
// You can then register tasks to use all loaded resources in Game::onStartup()
// These tasks are then executed with the given guarantees after the method returns
// ................................................................................

// Recommended and officially supported
using namespace magique;

// Declare your subclass of magique::Game to create a game
struct Demo final : Game
{

    // Override the onStartup() method to get access to the automatic asset loader
    void onStartup(AssetLoader& loader) override
    {
        // Use lambda expressions to register small and simple tasks - just a "small anonymous function"
        // It gets passed a AssetContainer
        // https://stackoverflow.com/questions/7627098/what-is-a-lambda-expression-and-when-should-i-use-one
        auto loadTextFilesLambda = [](AssetContainer& assets)
        {
            // Use a lambda expression to - it gets passed the individual asset of the directory
            auto iterateAssetDirLambda = [](Asset asset)
            {
                // Here we know all assets are text files - but we can check to be sure
                if (asset.getExtension() != std::string(".txt"))
                {
                    LOG_ERROR("Non text file in text directory!");
                    return; // Don't process invalid asset
                }

                // Print the contents of the file - or do any other work with it
                std::string fileText{asset.data, (size_t)asset.size};
                printf("%s", fileText.c_str());
            };
            // Iterate the given directory recursively
            assets.iterateDirectory("textFiles", iterateAssetDirLambda);
        };

        // If it's not textures you can use the background thread for smoother loading
        loader.registerTask(loadTextFilesLambda, BACKGROUND_THREAD);

        auto loadTextureFiles = [](AssetContainer& assets)
        {
            // Get an asset by name
            Asset textureAsset = assets.getAsset("MyPicture.png");
            if (textureAsset.isValid() == false)
            {
                LOG_ERROR("Internal magique loading error - please report to https://github.com/gk646/magique/issues");
                return; // Don't process invalid asset
            }

            // Resources used throughout the game should be registered to the asset management system
            // This allows fast retrieval at any point
            handle resourceHandle = RegisterTexture(textureAsset);
            // Register the handle by string - later when accessing this uses compile time hash for very fast access (O(1))
            RegisterHandle(resourceHandle, textureAsset.getFileName(false));
        };

        // We load a texture thus we register it to execute on the main thread
        loader.registerTask(loadTextureFiles, MAIN_THREAD);
    }


    // Override the drawGame() method to draw custom contents to the screen each render tick
    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        // Get the hash at compile time - don't forget constexpr when explicitly declaring it
        uint32_t hash = GetHash("MyPicture");
        // Draw our loaded texture
        DrawRegion(GetTexture(hash), 0, 0);
    }
};

#endif // MAGIQUE_ASSET_LOADING_DEMO_H