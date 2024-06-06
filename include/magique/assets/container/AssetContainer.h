#ifndef ASSETCONTAINER_H
#define ASSETCONTAINER_H

#include <vector>
#include <functional>

//-----------------------------------------------
// Asset Container
//-----------------------------------------------
// .....................................................................
// This class stores all the assets and allows access to them during loading
// .....................................................................

namespace magique
{
    struct Asset final
    {
        const char* name; // Full path of the file - from the compile root
        int size;         // File size
        const char* data; // File data

        // Checks if the assets extension matches the given one e.g '.png' myTexture.png
        // Includes the dot
        bool hasExtension(const char* extension) const;
    };

    struct AssetContainer final
    {
        AssetContainer() = default;                           // Default constructor
        explicit AssetContainer(std::vector<Asset>&& assets); // Internal constructor
        AssetContainer& operator=(AssetContainer&& other) noexcept = default;
        ~AssetContainer();

        // IMPORTANT: All assets are named with their path from the asset content root
        // Example :
        // The texture:       resources/textures/player.png
        // Compile Image:     assets::CompileImage("../resources");
        // While loading:     RegisterTexture(assets.getAsset("textures/player.png");

        // Iterates the given directory and calls 'func' for all entries with the current asset
        // Iterates entries in numeric order if they are named as such e.g. 0.mp3, 1.mp3...
        // Pass an empty string to iterate all files
        // Relative to the compiled image root e.g res/player/idle - compile("./res") - iterate("player/idle");
        void iterateDirectory(const char* name, const std::function<void(const Asset&)>& func) const;

        // Retrieves an asset by its name
        // Assets are registered with their relative path from the asset image root
        const Asset& getAsset(const char* name) const;

    private:
        std::vector<Asset> assets; // Internal file list
    };
} // namespace magique

#endif //ASSETCONTAINER_H