// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <filesystem>
#include <cmath>
#include <raylib/raylib.h>
#include <cxutil/cxstring.h>

#include <magique/assets/container/AssetContainer.h>
#include <magique/assets/AssetPacker.h>
#include <magique/util/Compression.h>
#include <magique/util/Logging.h>
#include <magique/internal/Macros.h>

#include "internal/datastructures/VectorType.h"
#include "internal/utils/EncryptionUtil.h"

namespace fs = std::filesystem;

inline constexpr auto IMAGE_HEADER = "ASSET";
inline constexpr auto IMAGE_HEADER_COMPRESSED = "COMPR";

static void ScanDirectory(const fs::path& directory, magique::vector<fs::path>& pathList)
{
    const auto iter = fs::directory_iterator(directory);
    for (const auto& entry : iter)
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

static bool CreatePathList(const char* directory, magique::vector<fs::path>& pathList)
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
        ScanDirectory(dirPath, pathList);
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

namespace magique
{
    // Dangerous method - but just switches out pointers - outside memory management is (should be) sound
    static void UnCompressImage(char*& imageData, int& imageSize)
    {
        const auto* start = reinterpret_cast<const unsigned char*>(&imageData[5]);
        auto data = Decompress(start, imageSize);
        delete[] imageData;
        imageSize = data.getSize();
        imageData = (char*)data.getData();
        data.pointer = nullptr; // we switch out the pointer
    }

    static bool ParseImage(char*& imageData, int& imageSize, std::vector<Asset>& assets, const uint64_t encryptionKey)
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
            const char* titlePointer;
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

    bool LoadAssetImage(AssetContainer& assets, const char* path, const uint64_t encryptionKey)
    {
        if (!fs::exists(path)) // User cant use AssetContainer -> its empty
        {
            LOG_WARNING("No asset image at path:%s", path);
            return false;
        }
        const auto startTime = GetTime();

        // read file
        FILE* file = fopen(path, "rb");
        if (file != nullptr)
        {
            // Get size
            fseek(file, 0, SEEK_END);
            int imageSize = ftell(file);
            fseek(file, 0, SEEK_SET);

            // Read image
            auto imageData = new char[imageSize];
            fread(imageData, imageSize, 1, file);
            fclose(file);

            const int original = imageSize;
            const bool res = ParseImage(imageData, imageSize, assets.assets, encryptionKey);
            assets.nativeData = imageData;
            assets.sort();
            if (res)
            {
                const auto time = static_cast<int>(round((GetTime() - startTime) * 1000.0F)); // Round to millis
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

    static bool IsImageOutdated(const char* directory)
    {
        const auto path = fs::path(directory);
        if (fs::exists(path) && fs::is_directory(path)) // needs to exist and be a directory
        {
            const auto filePath = path / "index.magique";
            if (fs::exists(filePath))
            {
                FILE* file = fopen(filePath.generic_string().c_str(), "rb");
                if (!file)
                {
                    LOG_WARNING("Failed to read asset image index file");
                    return true;
                }

                // Read file
                char buff[64]{};
                fread(buff, 64, 1, file);
                fclose(file);

                // Detect format
                if (strncmp(buff, "MAGIQUE_INDEX_FILE", 18) != 0)
                {
                    LOG_WARNING("Failed to read asset image index file");
                    return true;
                }

                char* ptr = &buff[20]; // Skip format

                // Check assets
                cxstructs::str_skip_char(ptr, ':', 1);
                const int assets = cxstructs::str_parse_int(ptr);
                if (assets == -1) // empty
                    return true;

                // Check Size
                cxstructs::str_skip_char(ptr, ':', 1);
                const int size = cxstructs::str_parse_int(ptr);
                if (assets == 0 || size == 0) // parse error
                {
                    LOG_WARNING("Failed to read asset image index file");
                    return true;
                }

                // Check mode (only same mode allowed)
                cxstructs::str_skip_char(ptr, ':', 1);
                const int mode = cxstructs::str_parse_int(ptr);
#ifdef NDEBUG
                if (mode == 0) // Debug mode
                {
                    return true;
                }
#else
                if (mode == 1) // Release mode
                {
                    return true;
                }
#endif

                // Actually check size of all assets
                vector<fs::path> pathList;
                CreatePathList(path.string().c_str(), pathList);

                // Count mismatch
                if (pathList.size() != assets)
                    return true;

                int totalSize = 0;
                for (const auto& assetPath : pathList)
                {
                    totalSize += static_cast<int>(fs::file_size(assetPath));
                }
                // Now only if the size is equal we dont regenerate it!
                return totalSize != size;
            }
        }
        return true;
    }

    static void CreateIndexFile(const char* directory, int asset, const int totalSize)
    {
        char filePath[512] = {0};
        snprintf(filePath, sizeof(filePath), "%s/index.magique", directory);

        FILE* file = fopen(filePath, "wb");
        if (!file)
        {
            LOG_WARNING("Failed to create index file: %s", filePath);
            return;
        }
        if (asset == 0)
            asset = -1; // empty

        char buffer[128] = {0}; // Buffer to hold the content
        const auto* fmt = "MAGIQUE_INDEX_FILE\nASSETS:%d\nSIZE:%d\nRelease:%d\n";
#ifdef NDEBUG // Release mode
        snprintf(buffer, sizeof(buffer), fmt, asset, totalSize, 1);
#else
        snprintf(buffer, sizeof(buffer), fmt, asset, totalSize, 0);
#endif
        const size_t bytesWritten = fwrite(buffer, sizeof(char), strlen(buffer), file);
        if (bytesWritten != strlen(buffer))
        {
            LOG_WARNING("Failed to write to index file: %s", filePath);
        }
        fclose(file);
    }

    static void WriteImage(const uint64_t encryptionKey, const vector<fs::path>& pathList, int& writtenSize,
                           const fs::path& rootPath, FILE* imageFile, vector<char> data)
    {
        std::string relativePathStr;
        int totalFileSize = 0; // Only raw file size
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
            totalFileSize += fileSize;
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
        CreateIndexFile(rootPath.string().c_str(), pathList.size(), totalFileSize);
    }

    bool CompileAssetImage(const char* directory, const char* fileName, const uint64_t encryptionKey,
                           const bool compress)
    {
        if (!IsImageOutdated(directory))
            return true;
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
            auto comp = Compress(reinterpret_cast<const unsigned char*>(data.data()), writtenSize);

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

            const auto time = static_cast<int>(round((GetTime() - startTime) * 1000.0F)); // Round to millis
            auto* logText = "Successfully compiled %s into %s | Took %d millis | Compressed: %.2f mb -> %.2f mb "
                            "(%.0f%%) | Assets: %d";
            LOG_INFO(logText, directory, fileName, time, writtenSize / 1'000'000.0F, comp.getSize() / 1'000'000.0F,
                     static_cast<float>(comp.getSize()) / writtenSize * 100.0F, size);
        }
        else
        {
            fclose(imageFile);
            const auto time = static_cast<int>(round((GetTime() - startTime) * 1000.0F)); // Round to millis
            auto* logText = "Successfully compiled %s into %s | Took %d millis | Total Size: %.2f mb | Assets: %d";
            LOG_INFO(logText, directory, fileName, time, writtenSize / 1'000'000.0F, size);
        }
        return true;
    }

    // https://en.wikipedia.org/wiki/MD5#Pseudocode
    Checksum GetAssetImageChecksum(const char* path)
    {
        MAGIQUE_ASSERT(path != nullptr, "Passed null");
        Checksum checksum{};
        if (!fs::exists(path))
        {
            LOG_WARNING("No asset image at path:%s/%s", GetWorkingDirectory(), path);
            return checksum;
        }

        checksum.first = 0x67452301;
        checksum.second = 0xefcdab89;
        checksum.third = 0x98badcfe;
        checksum.fourth = 0x10325476;

        auto processChunk = [](uint32_t(&M)[16], Checksum& checksum)
        {
            static constexpr unsigned int s[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                                                 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
                                                 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                                                 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

            static constexpr unsigned int K[] = {
                0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
                0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
                0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
                0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
                0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
                0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
                0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
                0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

            uint32_t a = checksum.first;
            uint32_t b = checksum.second;
            uint32_t c = checksum.third;
            uint32_t d = checksum.fourth;

            for (int i = 0; i < 64; i++)
            {
                uint32_t f = 0;
                uint32_t g = 0;

                if (i < 16)
                {
                    f = (b & c) | ((~b) & d);
                    g = i;
                }
                else if (i < 32)
                {
                    f = (d & b) | ((~d) & c);
                    g = (5 * i + 1) % 16;
                }
                else if (i < 48)
                {
                    f = b ^ c ^ d;
                    g = (3 * i + 5) % 16;
                }
                else
                {
                    f = c ^ (b | (~d));
                    g = (7 * i) % 16;
                }


                const uint32_t temp = d;
                d = c;
                c = b;
                b = b + ROTATE_LEFT((a + f + K[i] + M[g]), s[i]);
                a = temp;
            }

            checksum.first += a;
            checksum.second += b;
            checksum.third += c;
            checksum.fourth += d;
        };


        FILE* file = fopen(path, "rb");
        uint32_t chunk[16]{};

        fseek(file, 0, SEEK_END);
        const auto fileSize = static_cast<uint64_t>(ftell(file));
        fseek(file, 0, SEEK_SET);
        uint64_t totalBytesRead = 0;
        uint64_t bytesRead = 0;

        while (true)
        {
            bytesRead = fread(chunk, 1, 64, file);
            totalBytesRead += bytesRead;

            if (bytesRead == 64)
                processChunk(chunk, checksum);
            else
                break;
        }

        const uint64_t bitLength = fileSize * 8; // Message length in bits

        auto* byteChunk = reinterpret_cast<uint8_t*>(chunk);

        byteChunk[bytesRead] = 0x80;
        bytesRead++;

        if (bytesRead > 56) // Not enough space, need an extra block
        {
            // Zero pad the rest of the current block
            memset(byteChunk + bytesRead, 0, 64 - bytesRead);
            processChunk(chunk, checksum);

            memset(chunk, 0, 64);
            memcpy(byteChunk + 56, &bitLength, 8);
            processChunk(chunk, checksum);
        }
        else // Enough space to append the length
        {
            memset(byteChunk + bytesRead, 0, 56 - bytesRead);
            memcpy(byteChunk + 56, &bitLength, 8);
            processChunk(chunk, checksum);
        }

        fclose(file);

        return checksum;
    }

    bool ValidateAssetImage(const Checksum checksum, const char* path)
    {
        MAGIQUE_ASSERT(checksum != Checksum{}, "Passed empty checksum");
        return GetAssetImageChecksum(path) == checksum;
    }

} // namespace magique