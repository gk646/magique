#ifndef MAGIQUE_ASSETCONTAINER_H
#define MAGIQUE_ASSETCONTAINER_H

#include <vector>
#include <functional>

//-----------------------------------------------
// Asset Container
//-----------------------------------------------
// ................................................................................
// This class stores all the assets and allows access to them during loading
// ................................................................................

namespace magique
{
    struct Asset final
    {
        const char* path; // Full path of the file - from the compile root
        int size;         // File size
        const char* data; // File data

        // Checks if the assets extension matches the given one e.g '.png' myTexture.png
        // Includes the dot
        bool hasExtension(const char* extension) const;

        // True if the asset path starts with the given prefix
        bool startsWith(const char* prefix) const;

        // True if the asset path ends with the given prefix
        bool endsWith(const char* suffix) const;

        // True if the asset path contains the given sequence anywhere
        bool contains(const char* str) const;

        //----------------- GETTERS -----------------//
        // IMPORTANT: The string returned by these methods is only correct until any of these methods are called again

        // Returns a the direct file name without the asset - This means all characters after the last separator ("/")
        // Failure: returns nullptr if the asset's path is empty or no filename can be found
        //      extension - include file extension or not (".png",".wav", ...)
        [[nodiscard]] const char* getFileName(bool extension = true) const;

        // Returns the file extension of the asset - This means all characters after the last dot (".")
        // Failure: returns nullptr if the asset's path is empty or has no extension
        [[nodiscard]] const char* getExtension() const;

    private:
        inline static char stringBuffer[64]{}; // Shared string buffer for string returns
    };

    struct AssetContainer final
    {
        AssetContainer() = default;                                          // Default constructor
        AssetContainer(const char* nativeData, std::vector<Asset>&& assets); // Internal constructor
        AssetContainer& operator=(AssetContainer&& other) noexcept;
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
        const char* nativeData = nullptr; // Pointer to all the file data
        std::vector<Asset> assets;        // Internal file list
    };
} // namespace magique

#endif //MAGIQUE_ASSETCONTAINER_H