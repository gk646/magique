#include <fstream>
#include <filesystem>

#include <cxutil/cxtime.h>

#include <magique/assets/AssetPacker.h>
#include <magique/assets/container/AssetContainer.h>
#include <magique/util/Logging.h>

namespace fs = std::filesystem;

inline constexpr auto IMAGE_HEADER = "ASSET";
inline constexpr auto IMAGE_HEADER_COMPRESSED = "COMPR";

namespace
{
    void SymmetricEncrypt(char* data, const uint32_t size, const uint64_t key)
    {
        for (uint32_t i = 0; i < size; ++i)
        {
            data[i] ^= static_cast<char>(key >> i % 8 * 8);
        }
    }

    bool LoadImageFromMemory(char* imageData, uint32_t imageSize, std::vector<magique::Asset>& assets,
                             const uint64_t encryptionKey)
    {
        uint32_t totalSize = 0;
        uint32_t filePointer = 0;
        uint32_t totalEntries = 0;

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
            // Get title pointe
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

    void ScanDirectory(const fs::path& directory, std::vector<fs::path>& pathList)
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

    bool CreatePathList(const char* directory, std::vector<fs::path>& pathList)
    {
        fs::path dirPath(directory);
        std::error_code ec;
        fs::file_status status = fs::status(dirPath, ec);

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
} // namespace

namespace magique
{
    bool LoadAssetImage(const char* path, AssetContainer& assets, const uint64_t encryptionKey)
    {
        if (!std::filesystem::exists(path))
        {
            LOG_WARNING("No asset image at path: %s", path);
            return false;
        }
        cxstructs::now();
        std::ifstream file(path, std::ios::binary);
        if (file)
        {
            const uint32_t imageSize = fs::file_size(path);
            const auto fileData = new char[imageSize];
            file.read(fileData, imageSize);
            file.close();
            std::vector<Asset> assetList;
            const bool res = LoadImageFromMemory(fileData, imageSize, assetList, encryptionKey);
            assets = AssetContainer{fileData, std::move(assetList)};
            if (res)
            {
                LOG_INFO("Successfully loaded image %s - Took: %lld millis. Total Size: %.2f mb", path,
                         cxstructs::getTime<std::chrono::milliseconds>(), imageSize / 1'000'000.0F);
                return true;
            }
            LOG_ERROR("Failed to load asset image: %s", path);
            return false;
        }
        LOG_ERROR("Failed to load file: %s", path);
        file.close();
        return false;
    }

    inline void WriteData() {}

    void WriteImage(const uint64_t encryptionKey, const std::vector<fs::path>& pathList, int& writtenSize,
                    const fs::path& rootPath, FILE* imageFile, std::vector<char>& data)
    {
        for (auto& entry : pathList)
        {
            FILE* file = fopen(entry.generic_string().c_str(), "rb");
            if (!file)
            {
                LOG_ERROR("Could not open input file: %s", entry.generic_string().c_str());
                continue;
            }
            setvbuf(file, nullptr, _IONBF, 0);
            // Read file
            {
                fseek(file, 0, SEEK_END);
                const int fileSize = ftell(file);
                if (fileSize > data.size())
                    data.resize(fileSize, 0);
                fseek(file, 0, SEEK_SET);
                fread(data.data(), fileSize, 1, file);
                SymmetricEncrypt(data.data(), fileSize, encryptionKey);
                {
                    fs::path relativePath = fs::relative(entry, rootPath);
                    std::string relativePathStr = relativePath.generic_string();
                    auto pathLen = static_cast<int>(relativePathStr.size() + 1);
                    fwrite(&pathLen, sizeof(int), 1, imageFile);
                    fwrite(relativePathStr.c_str(), pathLen, 1, imageFile);
                    fwrite(&fileSize, sizeof(int), 1, imageFile);
                    fwrite(data.data(), fileSize, 1, imageFile);

                    writtenSize += fileSize;
                    writtenSize += pathLen;
                    writtenSize += 8; // two 4 byte ints
                }
            }
            fclose(file);
        }
    }
    bool CompileImage(const char* directory, const char* fileName, const uint64_t encryptionKey, const bool compress)
    {
        cxstructs::now();
        std::vector<fs::path> pathList;
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

        // Open target file
        int writtenSize = 0;
        const fs::path rootPath(directory);
        FILE* imageFile = fopen(fileName, "wb");
        if (!imageFile)
        {
            LOG_ERROR("Could not open file for writing: %s", fileName);
            return false;
        }
        setvbuf(imageFile, nullptr, _IONBF, 0);

        // Header
        if (compress)
        {
            fwrite(IMAGE_HEADER_COMPRESSED, strlen(IMAGE_HEADER_COMPRESSED), 1, imageFile);
        }
        else
        {
            fwrite(IMAGE_HEADER, strlen(IMAGE_HEADER), 1, imageFile);
        }
        fwrite(&writtenSize, 4, 1, imageFile);

        // write element count
        const int size = static_cast<int>(pathList.size());
        fwrite(&size, 4, 1, imageFile);
        writtenSize += static_cast<int>(strlen(IMAGE_HEADER)) + 4 + 4;

        std::vector<char> data;
        data.reserve(10000);

        // Write to file
        WriteImage(encryptionKey, pathList, writtenSize, rootPath, imageFile, data);

        // Write the total image size
        fseek(imageFile, 5, SEEK_SET);
        fwrite(&writtenSize, sizeof(int), 1, imageFile);

        // Close and log
        fclose(imageFile);
        LOG_INFO("Successfully compiled %s into %s - Took %lld millis. Total Size: %.2f mb", directory, fileName,
                 cxstructs::getTime<std::chrono::milliseconds>(), writtenSize / 1'000'000.0F);
        return true;
    }
} // namespace magique