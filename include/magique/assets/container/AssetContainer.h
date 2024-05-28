#ifndef ASSETCONTAINER_H
#define ASSETCONTAINER_H

#include <vector>

struct Asset
{
    const char* name; // Full name of the file - from the compile root
    uint32_t size;    // File size
    const char* data; // File data
};


struct AssetContainer
{
    std::vector<Asset> assets; // Internal file list

    AssetContainer();
    ~AssetContainer();

    void sort();
};


#endif //ASSETCONTAINER_H