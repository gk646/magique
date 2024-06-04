#ifndef ASSETCONTAINER_H
#define ASSETCONTAINER_H

#include <vector>

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
        const char* name; // Full name of the file - from the compile root
        int size;         // File size
        const char* data; // File data
        [[nodiscard]] const unsigned char* getData() const { return reinterpret_cast<const unsigned char*>(data); }
    };

    using LoadFunc = void (*)(const Asset& asset);

    struct AssetContainer final
    {
        AssetContainer() = default;                           // Default constructor
        explicit AssetContainer(std::vector<Asset>&& assets); // Internal constructor
        AssetContainer& operator=(AssetContainer&& other) noexcept =  default;
        ~AssetContainer();

        // Iterates the given directory and calls 'func' for all entries
        // Iterates entries in numeric order if they are named as such e.g. 0.mp3, 1.mp3...
        // Failure: returns false
        bool IterateDirectory(const char* name, LoadFunc func) const;

        // Retrieves an asset by its name
        // Assets are registered with their relative path from the asset image root
        const Asset& GetAsset(const char* name) const;

    private:
        std::vector<Asset> assets; // Internal file list
    };
} // namespace magique

#endif //ASSETCONTAINER_H