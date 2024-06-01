#include <magique/assets/AssetPacker.h>
#include <magique/core/DataStructures.h>
#include <magique/assets/container/AssetContainer.h>
#include <magique/util/Logging.h>

#include <cxutil/cxstring.h>
#include <cxutil/cxtime.h>

#include <fstream>
#include <filesystem>

// Original implementation is heavily modified
/*
    Modified version of the Simple-Asset-Packer by:
    MIT License

    Copyright (c) 2020 Hugh Nixon

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
 */

namespace fs = std::filesystem;

inline constexpr auto IMAGE_HEADER = "ASSET";

namespace
{
    void SymmetricEncrypt(char* data, const uint32_t size, const uint64_t key)
    {
        for (uint32_t i = 0; i < size; ++i)
        {
            data[i] ^= static_cast<char>(key >> i % 8 * 8);
        }
    }

    bool LoadImageFromMemory(const char* imageData, uint32_t imageSize, magique::AssetContainer& assets,
                             uint64_t encryptionKey)
    {
        uint32_t totalSize = 0;
        uint32_t filePointer = 0;

        if (memcmp(IMAGE_HEADER, &imageData[filePointer], 5) != 0)
        {
            LOG_ERROR("Malformed asset image File");
            return false;
        }

        filePointer += 5;
        memcpy(&totalSize, &imageData[filePointer], 4);

        if (imageSize == 0)
            imageSize = totalSize;

        if (imageSize != totalSize)
        {
            LOG_ERROR("Image size mismatch between header and buffer size");
            return false;
        }
        // Skip header file size
        filePointer += 4;
        constexpr int MAX_TITLE_LENGTH = 256;
        char titleBuffer[MAX_TITLE_LENGTH];

        while (filePointer < imageSize && filePointer < totalSize)
        {
            uint32_t titleLen = 0;
            uint32_t fileSize = 0;
            memset(titleBuffer, 0, MAX_TITLE_LENGTH);
            // Copy title
            {
                memcpy(&titleLen, &imageData[filePointer], 4);
                if (titleLen > MAX_TITLE_LENGTH)
                {
                    LOG_ERROR("Filename exceeds limit");
                    return false;
                }
                filePointer += 4;
                memcpy(titleBuffer, &imageData[filePointer], titleLen);
                filePointer += titleLen;
            }

            // Copy file data
            memcpy(&fileSize, &imageData[filePointer], 4);
            if (fileSize > imageSize - filePointer)
            {
                LOG_ERROR("File data exceeds image data");
                return false;
            }
            filePointer += 4;
            auto* fileData = new char[fileSize];
            memcpy(fileData, &imageData[filePointer], fileSize);
            SymmetricEncrypt(fileData, fileSize, encryptionKey);
            filePointer += fileSize;

            assets.assets.push_back({cxstructs::str_dup(titleBuffer), fileSize, fileData});
        }
        return true;
    }

    void ScanDirectory(const fs::path& directory, magique::vector<fs::path>& pathList)
    {
        for (const auto& entry : fs::directory_iterator(directory))
        {
            if (entry.is_directory())
            {
                ScanDirectory(entry.path(), pathList);
            }
            else if (entry.is_regular_file())
            {
                pathList.push_back(entry.path());
            }
        }
    }

    bool CreatePathList(const char* directory, magique::vector<fs::path>& pathList)
    {
        if (fs::exists(directory))
        {
            if (fs::is_directory(directory))
            {
                ScanDirectory(fs::directory_entry(directory), pathList);
                return true;
            }
            if (fs::is_regular_file(directory))
            {
                pathList.emplace_back(directory);
                return true;
            }
            LOG_ERROR("Error: Given path is not directory of file: %s", directory);
            return false;
        }
        LOG_ERROR("Error: Given directory does not exist: %s", directory);
        return false;
    }

} // namespace

namespace magique::assets
{

    bool LoadAssetImage(const char* path, AssetContainer& assets, const uint64_t encryptionKey)
    {
        if (!std::filesystem::exists(path))
        {
            LOG_WARNING("Given path does not exist: %s", path);
            return false;
        }
        cxstructs::now();
        assets.assets.reserve(100);
        std::ifstream file(path, std::ios::binary);
        if (file)
        {
            const uint32_t imageSize = fs::file_size(path);
            const auto fileData = new char[imageSize];
            file.read(fileData, imageSize);
            file.close();
            const bool res = LoadImageFromMemory(fileData, imageSize, assets, encryptionKey);
            delete[] fileData;
            if (res)
            {
                LOG_INFO("Successfully loaded image %s - Took: %lld millis. Total Size: %d\n", path,
                         cxstructs::getTime<std::chrono::milliseconds>(), imageSize);
                return true;
            }
            LOG_ERROR("Failed to load asset image: %s", path);
            return false;
        }
        LOG_ERROR("Failed to load file: %s", path);
        file.close();
        return false;
    }


    bool CompileImage(const char* directory, const char* fileName, const uint64_t encryptionKey)
    {
        cxstructs::now();
        vector<fs::path> pathList;
        pathList.reserve(100);

        if (!CreatePathList(directory, pathList))
        {
            return false;
        }

        if (pathList.empty())
        {
            LOG_ERROR("No files to compile into asset image");
            return false;
        }

        fs::path rootPath(directory);
        std::ofstream image(fileName, std::ios::binary);

        if (!image)
        {
            LOG_ERROR("Could not open file for writing: %s", fileName);
            return false;
        }

        image.write(IMAGE_HEADER, strlen(IMAGE_HEADER));
        image.write("\x00\x00\x00\x00", 4);

        for (auto& entry : pathList)
        {
            std::ifstream file(entry, std::ios::binary | std::ios::ate);
            if (!file)
            {
                LOG_ERROR("Could not open input file: %s", entry.generic_string().c_str());
                continue;
            }

            // Read file
            {
                std::ifstream::pos_type fileSize = file.tellg();
                file.seekg(0, std::ios::beg);
                auto* fileData = new char[fileSize];
                file.read(fileData, fileSize);
                SymmetricEncrypt(fileData, fileSize, encryptionKey);

                {
                    auto size = static_cast<uint32_t>(fileSize);
                    fs::path relativePath = fs::relative(entry, rootPath);
                    std::string relativePathStr = relativePath.generic_string();
                    auto pathLen = static_cast<uint32_t>(relativePathStr.size());
                    image.write(reinterpret_cast<const char*>(&pathLen), sizeof(pathLen));
                    image.write(relativePathStr.c_str(), pathLen);
                    image.write(reinterpret_cast<const char*>(&size), sizeof(size));
                    image.write(fileData, size);
                }
                delete[] fileData;
            }
            file.close();
        }

        uint32_t writtenSize = image.tellp();
        image.seekp(5);
        image.write(reinterpret_cast<const char*>(&writtenSize), sizeof(writtenSize));
        image.close();
        LOG_INFO("Successfully compiled %s into %s - Took %lld millis. Total Size: %d\n", directory, fileName,
                 cxstructs::getTime<std::chrono::milliseconds>(), writtenSize);
        return true;
    }

} // namespace magique::assets