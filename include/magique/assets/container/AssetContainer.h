// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSET_CONTAINER_H
#define MAGIQUE_ASSET_CONTAINER_H

#include <vector>
#include <magique/fwd.hpp>
#include <magique/assets/types/Asset.h>
#include <magique/internal/PlatformIncludes.h>

//===============================================
// Asset Container
//===============================================
// ................................................................................
// This class stores assets and allows structured access to them
// ................................................................................

namespace magique
{
    struct AssetContainer final
    {
        // IMPORTANT: All assets are named with their path from the asset content root
        // Example :
        // The texture:       resources/textures/player.png
        // Compile Image:     assets::CompileImage("../resources");
        // While loading:     RegisterTexture(assets.getAsset("textures/player.png");

        // Iterates the given directory and calls 'func' for all entries with the current asset
        // Iterates entries in numeric order if they are named as such e.g. 0.mp3, 1.mp3...
        // Pass an empty string to iterate all files
        // Relative to the compiled image root e.g. res/player/idle - compile("./res") - iterate("player/idle");
        void iterateDirectory(const char* directory, const std::function<void(Asset asset)>& func) const;

        // Retrieves the first asset that matches the given path
        // This is a fast operation  - O (log n)
        const Asset& getAssetByPath(const char* path) const;

        // Retrieves the first asset that matches the given name
        // This is slower than ByPath - O (n)
        const Asset& getAsset(const char* name) const;

        // Returns the total amount of assets
        [[nodiscard]] int getSize() const;

        ~AssetContainer();

    private:
        AssetContainer() = default;
        void sort();
        const char* nativeData = nullptr; // Pointer to all the file data
        std::vector<Asset> assets;        // Internal file list
        friend struct AssetLoader;
        friend bool LoadAssetImage(AssetContainer&, const char*, uint64_t);
    };

} // namespace magique

#endif //MAGIQUE_ASSET_CONTAINER_H