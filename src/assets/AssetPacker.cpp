
#include "AssetPacker.h"
#include <magique/assets/AssetManager.h>

#include <iostream>
#include <fstream>
#include <utility>
#include <filesystem>


int* allocate()
{
    return new int();
}

namespace AssetPacker {

void deleteFileImageMap(fileImageMap_t* fileImageMap) {
  for (auto& i : *fileImageMap)
    i.second.free();
}

auto loadImageFromMemory(const char* imageData, uint32_t imageSize, fileImageMap_t& files) -> bool {
  uint32_t totalSize = 0;
  uint32_t filePointer = 0;
  if (memcmp("ASSET", &imageData[filePointer], 5) != 0) {
    std::cout << "Malformed Image File" << std::endl;
    return false;
  }

  filePointer += 5;
  memcpy(&totalSize, &imageData[filePointer], 4);
  std::cout << "Image Size: " << totalSize << " bytes given, " << imageSize << " read." << std::endl;

  if (imageSize == 0) imageSize = totalSize;

  if (imageSize != totalSize) {
    std::cout << "Image size mismatch between header and buffer size" << std::endl;
    return false;
  }

  filePointer += 4;

  while (filePointer < imageSize && filePointer < totalSize) {
    uint32_t titleLen = 0;
    uint32_t fileSize = 0;
    char title[256];
    memset(title, 0, 256);

    memcpy(&titleLen, &imageData[filePointer], 4);
    if (titleLen > 256) {
      std::cout << "Image filename exceeds limit" << std::endl;
      return false;
    }
    filePointer += 4;
    memcpy(title, &imageData[filePointer], titleLen);
    filePointer += titleLen;
    memcpy(&fileSize, &imageData[filePointer], 4);
    if (fileSize > imageSize - filePointer) {
      std::cout << "Image file size exceeds container" << std::endl;
      return false;
    }
    filePointer += 4;
    char* LocalFileData = new char[fileSize];
    memcpy(LocalFileData, &imageData[filePointer], fileSize);
    filePointer += fileSize;

    files[title] = FileImage(title, fileSize, LocalFileData);
  }
  return true;
}

auto findImageInFile(const fs::path& filePath, fileImageMap_t& files, bool findLastImageInFile) -> bool {
  std::ifstream file(filePath, std::ios::binary);
  if (file) {
    uint32_t fileSize = fs::file_size(filePath);
    char* fileData = new char[fileSize];
    file.read(fileData, fileSize);
    file.close();
    if (findLastImageInFile) {
      for (int i = fileSize - 16; i > 0; --i) {
        if (memcmp(fileData + i, "ASSET", 5) == 0) {
          if (loadImageFromMemory(fileData + i, 0, files)) return true;
        }
      }
    }
    for (int i = 0; i < fileSize - 16; ++i) {
      if (memcmp(fileData + i, "ASSET", 5) == 0) {
        if (loadImageFromMemory(fileData + i, 0, files)) return true;
      }
    }

    delete[] fileData;
    return false;
  }
  std::cout << "Failed to load image." << std::endl;
  file.close();
  return false;
}

auto loadImage(const fs::path& imagePath, fileImageMap_t& files) -> bool {
  std::ifstream file(imagePath, std::ios::binary);
  if (file) {
    uint32_t imageSize = fs::file_size(imagePath);
    auto fileData = new char[imageSize];
    file.read(fileData, imageSize);
    file.close();
    const bool ret = loadImageFromMemory(fileData, imageSize, files);
    delete[] fileData;
    return ret;
  }
  std::cout << "Failed to load image." << std::endl;
  file.close();
  return false;
}

bool appendImageToFile(const fs::path& imagePath, const fs::path& filePath) {
  std::ofstream file(filePath, std::ios::binary | std::ios::ate | std::ios::app);
  if (file) {
    uint32_t fileSize = fs::file_size(filePath);
    uint32_t imageFileSize = fs::file_size(imagePath);
    std::ifstream imageFile(imagePath, std::ios::binary);
    if (imageFile) {
      char* imageFileData = new char[imageFileSize];
      imageFile.read(imageFileData, imageFileSize);
      imageFile.close();
      file.write(imageFileData, imageFileSize);
      file.write((char*)&fileSize, 4);
      file.close();

      delete[] imageFileData;
      return true;
    } else {
      std::cout << "Cannot open file: " << imagePath << std::endl;
      imageFile.close();
      file.close();
      return false;
    }
  } else {
    std::cout << "Cannot open file: " << filePath << std::endl;
    file.close();
    return false;
  }
}

FileImage::FileImage() {
  data = nullptr;
  size = 0;
  path = "";
}

FileImage::FileImage(std::string path, uint32_t size, const char* data)
    : path(std::move(path)), size(size), data(data) {}

void FileImage::free() {
  delete[] data;
  data = nullptr;
}
}  // namespace AssetPacker