#ifndef MAGIQUE_ASSETCONTAINER_H
#define MAGIQUE_ASSETCONTAINER_H

#include <vector>
#include <magique/assets/types/Asset.h>

//-----------------------------------------------
// Asset Container
//-----------------------------------------------
// ................................................................................
// This class stores all the assets and allows access to them during loading
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
        // Relative to the compiled image root e.g res/player/idle - compile("./res") - iterate("player/idle");
        void iterateDirectory(const char* directory, const std::function<void(const Asset&)>& func) const;

        // Retrieves the first asset that matches the given path
        // This is fast operation
        const Asset& getAssetByPath(const char* path) const;

        // Retrieves the first asset that matches the given
        // This is slower than ByPath
        const Asset& getAsset(const char* name) const;

        // Returns the total amount of assets
        [[nodiscard]] int getSize() const;

    private:
        AssetContainer() = default;
        AssetContainer(const char* nativeData, std::vector<Asset>&& assets);
        AssetContainer& operator=(AssetContainer&& other) noexcept;
        ~AssetContainer();

        const char* nativeData = nullptr; // Pointer to all the file data
        std::vector<Asset> assets;        // Internal file list
    };
} // namespace magique

#endif //MAGIQUE_ASSETCONTAINER_H