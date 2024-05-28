#pragma once

#include "fwd.h"

namespace fs = std::filesystem;

namespace magique
{

    void deleteFileImageMap(fileImageMap_t* fileImageMap);
    bool makeImage(const fs::path& imageName, pathList_t& files);
    bool findImageInFile(const fs::path& filePath, fileImageMap_t& files, bool findLastImageInFile = 0);
    bool loadImageFromMemory(const char* imageData, uint32_t imageSize, fileImageMap_t& files);
    bool loadImage(const fs::path& imagePath, fileImageMap_t& files);
    bool appendImageToFile(const fs::path& imagePath, const fs::path& filePath);

} // namespace AssetPacker