#define _CRT_SECURE_NO_WARNINGS
#include <filesystem>
#include <raylib/raylib.h>

#include <magique/assets/AssetPacker.h>
#include <magique/assets/container/AssetContainer.h>
#include <magique/util/Compression.h>
#include <magique/util/Logging.h>

#include "internal/datastructures/VectorType.h"
#include "internal/headers/Security.h"

namespace fs = std::filesystem;

inline constexpr auto IMAGE_HEADER = "ASSET";
inline constexpr auto IMAGE_HEADER_COMPRESSED = "COMPR";

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
    fs::path dirPath(directory);
    std::error_code ec;
    const fs::file_status status = fs::status(dirPath, ec);

    if (ec)
    {
        LOG_ERROR("Error: Cannot access path: %s", directory);
        return false;
    }

    if (fs::is_directory(status))
    {
        ScanDirectory(fs::directory_entry(dirPath), pathList);
        return true;
    }
    if (fs::is_regular_file(status))
    {
        pathList.emplace_back(dirPath);
        return true;
    }
    LOG_ERROR("Error: Given path is not directory or file: %s", directory);
    return false;
}

void UnCompressImage(char*& imageData, int& imageSize)
{
    const auto* start = (const unsigned char*)&imageData[5];
    auto data = magique::DeCompress(start, imageSize);
    delete[] imageData;
    imageSize = data.getSize();
    imageData = (char*)data.getData();
}

namespace magique
{
    bool ParseImage(char*& imageData, int& imageSize, std::vector<Asset>& assets, const uint64_t encryptionKey)
    {
        int totalSize = 0;
        int filePointer = 0;
        int totalEntries = 0;

        if (memcmp(IMAGE_HEADER, &imageData[filePointer], 5) != 0)
        {
            if (memcmp(IMAGE_HEADER_COMPRESSED, &imageData[filePointer], 5) == 0)
            {
                UnCompressImage(imageData, imageSize);
            }
            else
            {
                LOG_ERROR("Malformed asset image File");
                return false;
            }
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

        memcpy(&totalEntries, &imageData[filePointer], 4);

        if (totalEntries < 50'000 && totalEntries > 0) // Sanity check
        {
            assets.reserve(totalEntries + 1);
        }

        filePointer += 4;

        while (filePointer < imageSize && filePointer < totalSize)
        {
            int titleLen = 0;
            int fileSize = 0;
            const char* titlePointer = nullptr;
            // Get title pointer
            {
                memcpy(&titleLen, &imageData[filePointer], 4);
                if (titleLen > 512)
                {
                    LOG_ERROR("Filename exceeds limit");
                    return false;
                }
                filePointer += 4;
                titlePointer = &imageData[filePointer];
                filePointer += titleLen;
            }

            // Get file pointer
            memcpy(&fileSize, &imageData[filePointer], 4);
            if (fileSize > imageSize - filePointer)
            {
                LOG_ERROR("File data exceeds image data");
                return false;
            }
            filePointer += 4;
            SymmetricEncrypt(&imageData[filePointer], fileSize, encryptionKey);
            assets.push_back({titlePointer, fileSize, &imageData[filePointer]});
            filePointer += fileSize;
        }
        return true;
    }

    bool LoadAssetImage(const char* path, AssetContainer& assets, const uint64_t encryptionKey)
    {
        if (!std::filesystem::exists(path))
        {
            LOG_WARNING("No asset image at path: %s", path);
            return false;
        }
        const auto startTime = GetTime();

        // read file
        FILE* file = fopen(path, "rb");
        if (file)
        {
            // Get size
            fseek(file, 0, SEEK_END);
            int imageSize = ftell(file);
            fseek(file, 0, SEEK_SET);

            // Read image
            auto imageData = new char[imageSize];
            fread(imageData, imageSize, 1, file);
            fclose(file);

            int original = imageSize;
            std::vector<Asset> assetList;
            const bool res = ParseImage(imageData, imageSize, assetList, encryptionKey);
            assets = AssetContainer{imageData, std::move(assetList)};
            if (res)
            {
                const auto time = static_cast<int>(std::round((GetTime() - startTime) * 1000.0F));
                if (original == imageSize)
                {
                    auto* logText =
                        "Successfully loaded asset image %s | Took: %d millis | Total Size: %.2f mb | Assets: %d";
                    LOG_INFO(logText, path, time, imageSize / 1'000'000.0F, assets.getSize());
                }
                else
                {
                    auto* logText = "Successfully loaded asset image %s | Took: %d millis. Decompressed: %.2f mb -> "
                                    "%.2f mb | Assets: %d";
                    LOG_INFO(logText, path, time, original / 1'000'000.0F, imageSize / 1'000'000.0F, assets.getSize());
                }
                return true;
            }
            LOG_ERROR("Failed to parse asset image: %s", path);
            return false;
        }
        LOG_ERROR("Failed to load asset image file: %s", path);
        return false;
    }

    void WriteImage(const uint64_t encryptionKey, const vector<fs::path>& pathList, int& writtenSize,
                    const fs::path& rootPath, FILE* imageFile, vector<char> data)
    {
        std::string relativePathStr;
        for (const auto& entry : pathList)
        {
            // Open the input file for reading
            FILE* file = fopen(entry.generic_string().c_str(), "rb");
            if (file == nullptr)
            {
                LOG_ERROR("Could not open input file: %s", entry.generic_string().c_str());
                continue;
            }
            setvbuf(file, nullptr, _IONBF, 0);

            // Read the file size
            fseek(file, 0, SEEK_END);
            const int fileSize = ftell(file);
            if (fileSize > data.size())
                data.resize(fileSize, 0);
            fseek(file, 0, SEEK_SET);

            // Read the file data into the buffer
            fread(data.data(), fileSize, 1, file);

            // Encrypt the file data
            SymmetricEncrypt(data.data(), fileSize, encryptionKey);

            // Get the relative path for the file
            fs::path relativePath = fs::relative(entry, rootPath);
            relativePathStr = relativePath.generic_string();
            auto pathLen = static_cast<int>(relativePathStr.size() + 1);

            // Write the path length to the image file
            fwrite(&pathLen, sizeof(int), 1, imageFile);

            // Write the relative path to the image file
            fwrite(relativePathStr.c_str(), pathLen, 1, imageFile);

            // Write the file size to the image file
            fwrite(&fileSize, sizeof(int), 1, imageFile);

            // Write the encrypted file data to the image file
            fwrite(data.data(), fileSize, 1, imageFile);

            // Update the total written size - Two 4 byte integers for path length and file size
            writtenSize += fileSize;
            writtenSize += pathLen;
            writtenSize += 8;

            // Close the input file
            fclose(file);
        }
    }

    bool CompileImage(const char* directory, const char* fileName, const uint64_t encryptionKey, const bool compress)
    {
        const auto startTime = GetTime();
        vector<fs::path> pathList;
        pathList.reserve(100);

        // Create the list of file paths
        if (!CreatePathList(directory, pathList))
        {
            return false;
        }
        if (pathList.empty())
        {
            LOG_ERROR("No files to compile into asset image");
            return false;
        }

        int writtenSize = 0;
        const fs::path rootPath(directory);

        // Open the image file for writing
        FILE* imageFile = fopen(fileName, "w+b");
        if (!imageFile)
        {
            LOG_ERROR("Could not open file for writing: %s", fileName);
            return false;
        }

        // Write the header
        fwrite(IMAGE_HEADER, strlen(IMAGE_HEADER), 1, imageFile);

        // Write total size
        fwrite(&writtenSize, 4, 1, imageFile);

        // Write elements
        const int size = pathList.size();
        fwrite(&size, 4, 1, imageFile);
        writtenSize += static_cast<int>(strlen(IMAGE_HEADER)) + 4 + 4;

        // Temp data from files
        vector<char> data;
        data.reserve(10000);

        // Write the image data
        WriteImage(encryptionKey, pathList, writtenSize, rootPath, imageFile, data);

        // Update the total written size in the file header
        fseek(imageFile, 5, SEEK_SET);
        fwrite(&writtenSize, sizeof(int), 1, imageFile);

        if (compress)
        {
            // Read the file data into a vector for compression
            if (writtenSize > data.size())
                data.resize(writtenSize);

            fseek(imageFile, 0, SEEK_SET);
            fread(data.data(), writtenSize, 1, imageFile);
            fclose(imageFile);

            // Compress the data
            auto comp = Compress((const unsigned char*)data.data(), writtenSize);

            // Open the file again to write the compressed data
            FILE* compFile = fopen(fileName, "wb");
            if (!compFile)
            {
                LOG_ERROR("Could not open file for writing: %s", fileName);
                return false;
            }

            // Write the compressed data to the file
            fwrite(IMAGE_HEADER_COMPRESSED, 5, 1, compFile);
            fwrite(comp.getData(), comp.getSize(), 1, compFile);
            comp.free();
            fclose(compFile);
            auto* logText = "Successfully compiled %s into %s | Took %lld millis | Compressed: %.2f mb -> %.2f mb "
                            "(%.0f%%) | Assets: %d";
            const auto time = static_cast<int>(std::round((GetTime() - startTime) * 1000.0F));
            LOG_INFO(logText, directory, fileName, time, writtenSize / 1'000'000.0F, comp.getSize() / 1'000'000.0F,
                     ((float)comp.getSize() / writtenSize) * 100.0F, size);
        }
        else
        {
            fclose(imageFile);
            auto* logText = "Successfully compiled %s into %s | Took %lld millis | Total Size: %.2f mb | Assets: %d";
            const auto time = static_cast<int>(std::round((GetTime() - startTime) * 1000.0F));
            LOG_INFO(logText, directory, fileName, time, writtenSize / 1'000'000.0F, size);
        }
        return true;
    }
} // namespace magique