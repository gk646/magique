#pragma once

#include <map>
#include <filesystem>
#include "fwd.h"

namespace fs = std::filesystem;

namespace AssetPacker {
typedef std::vector<std::filesystem::path> pathList_t;

extern void deleteFileImageMap(fileImageMap_t* fileImageMap);
extern bool makeImage(const fs::path& imageName, pathList_t& files);
extern bool findImageInFile(const fs::path& filePath, fileImageMap_t& files, bool findLastImageInFile = 0);
extern bool loadImageFromMemory(const char* imageData, uint32_t imageSize, fileImageMap_t& files);
extern bool loadImage(const fs::path& imagePath, fileImageMap_t& files);
extern bool appendImageToFile(const fs::path& imagePath, const fs::path& filePath);
}  // namespace AssetPacker