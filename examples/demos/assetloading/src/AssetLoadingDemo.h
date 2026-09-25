// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSET_LOADING_DEMO_H
#define MAGIQUE_ASSET_LOADING_DEMO_H

#include <magique/magique.hpp>

StringHashMap<TextureRegion> TEXTURES; // Name based lookup for simplicity

// Declare your subclass of magique::Game to create a game
struct Demo final : Game
{

    // Override the onStartup() method to get access to the asset loader
    void onStartup(AssetLoader& loader) override
    {
        // Use lambda expressions to register small and simple tasks - just a "small anonymous function"
        // It gets passed the asset pack
        // https://stackoverflow.com/questions/7627098/what-is-a-lambda-expression-and-when-should-i-use-one
        auto loadTextureFiles = [](AssetPack& assets)
        {
            // Get an asset by name
            Asset textureAsset = assets["MyPicture.png"];
            if (textureAsset.isValid() == false)
            {
                LOG_ERROR("Internal magique loading error - please report to https://github.com/gk646/magique/issues");
                return; // Don't process invalid asset
            }

            // Register the texture with its file name - without extension (.png)
            TEXTURES[textureAsset.getFileName(false)] = ImportTexture(textureAsset);
        };

        // Register the load task - texture access MUST be on the main thread
        loader.registerTask(loadTextureFiles, THREAD_MAIN);

        auto loadTextFilesLambda = [](AssetPack& assets)
        {
            // Iterate the given directory recursively
            // Lambdas can also be defined in line directly
            assets.forEachIn("textFiles",
                             [](Asset asset)
                             {
                                 // Here we know all assets are text files - but we can check to be sure
                                 if (asset.getExtension() != ".txt")
                                 {
                                     LOG_ERROR("Non text file in text directory!");
                                     return; // Don't process invalid asset
                                 }

                                 // Print the contents of the file - or do any other work with it
                                 LOG_INFO("%s", asset.getData().data());
                             });
        };

        // If it's not textures allow any thread to handle loading so the main thread is not blocked
        loader.registerTask(loadTextFilesLambda, THREAD_ANY);
    }

    void onDrawGame(GameState state, Camera2D& camera) override
    {
        DrawRegionCentered(TEXTURES["MyPicture"], GetScreenDims() / 2);
    }
};

#endif // MAGIQUE_ASSET_LOADING_DEMO_H
