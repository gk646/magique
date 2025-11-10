// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSET_CONTAINER_H
#define MAGIQUE_ASSET_CONTAINER_H

#include <vector>
#include <magique/fwd.hpp>
#include <magique/assets/types/Asset.h>
#include <magique/internal/Macros.h>
#include <magique/internal/PlatformIncludes.h>

//===============================================
// Asset Container
//===============================================
// ................................................................................
// This class stores assets and allows structured or direct access to them.
// You interact with it by registering a task in Game::onStartup() where its passed as argument.
//
// IMPORTANT: All assets are named with their path from the asset content root
// Example:
// The texture:       resources/textures/player.png
// Compile Image:     assets::CompileImage("../resources");
// While loading:     RegisterTexture(assets.getAsset("textures/player.png");
// ................................................................................

namespace magique
{
    struct AssetContainer final
    {
        // Iterates the given directory recursively and calls 'func' for all entries with the current asset
        // Iterates entries in numeric order if they are named as such e.g. 0.mp3, 1.mp3...
        // Pass an empty string to iterate all files
        // Relative to the compiled image root e.g. res/player/idle - compile("./res") - iterate("player/idle");
        void iterateDirectory(const char* directory, const std::function<void(Asset asset)>& func) const;

        // Retrieves the first asset that matches the given path - use getAsset() to do a easier name search
        // Note: This should be a path to a file - otherwise will return the first file that matches the given path
        //       e.g. music/Sound - can return - music/Sound/ambient/Animals.wav
        // This is a fast operation  - O (log n)
        const Asset& getAssetByPath(const char* path) const;

        // Retrieves the first asset that contains the given name
        // This is slower than getAssetByPath() but more convenient - O (n)
        // This does NOT require the full name - any fully mathing path will be returned
        // e.g. res/icons/currencies/human/gold.png will be found by: human/gold.png
        // Just make sure its still unique!
        const Asset& getAsset(const char* name) const;

        // Returns a reference to the asset vector containing all loaded assets
        const std::vector<Asset>& getAllAssets() const;

        // Returns the total amount of assets loaded
        int getSize() const;

        ~AssetContainer();

    private:
        M_MAKE_PUB()
        AssetContainer() = default;
        void sort();
        const char* nativeData = nullptr; // Pointer to all the file data
        std::vector<Asset> assets;        // Internal file list
        friend struct AssetLoader;
        friend bool LoadAssetImage(AssetContainer&, const char*, uint64_t);
    };

} // namespace magique

#endif //MAGIQUE_ASSET_CONTAINER_H