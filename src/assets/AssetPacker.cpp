// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <filesystem>
#include <vector>
#include <cstring>
#include <cmath>
#include <raylib/raylib.h>
#include <raylib/config.h>

#include <magique/assets/AssetPacker.h>
#include <magique/assets/AssetContainer.h>
#include <magique/util/Logging.h>

#include "internal/utils/EncryptionUtil.h"
#include "external/raylib/src/external/sinfl.h"
#include "external/raylib/src/external/sdefl.h"
#include "magique/util/Strings.h"

namespace fs = std::filesystem;

inline constexpr auto IMAGE_HEADER = "ASSET";
inline constexpr auto IMAGE_HEADER_COMPRESSED = "COMPR";
inline constexpr int HEADER_LEN = 5; // Including terminator

namespace magique
{
    using PathList = std::vector<fs::path>;

    bool ReadFile(std::string_view name, std::string& data)
    {
        FILE* file = fopen(name.data(), "rb");
        if (file == nullptr)
        {
            return false;
        }
        fseek(file, 0, SEEK_END);
        int fileSize = (int)ftell(file); // File size per file limited to 2GB
        data.resize(fileSize);
        fseek(file, 0, SEEK_SET);
        fread(data.data(), fileSize, 1, file);
        fclose(file);
        return true;
    }

    bool WriteFile(const char* fileName, std::string_view content)
    {
        FILE* file = fopen(fileName, "w+b");
        if (file == nullptr)
        {
            LOG_ERROR("Could not open file for writing: %s", fileName);
            return false;
        }

        fwrite(content.data(), content.size(), 1, file);
        fclose(file);
        return true;
    }

    static void ScanDirectory(const fs::path& directory, PathList& pathList)
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

    static bool CreatePathList(const char* directory, PathList& pathList)
    {
        fs::path dirPath(directory);
        std::error_code ec;
        const fs::file_status status = fs::status(dirPath, ec);
        if (ec)
        {
            LOG_ERROR("Error: Cannot access path: %s", directory);
            return false;
        }

        pathList.reserve(100);
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

    struct PackEntry final
    {
        int nameLen;
        const char* name;
        int dataLen;
        const char* data;

        PackEntry() = default;

        PackEntry(const char* name, const char* data, int dataLen) :
            nameLen(strlen(name)), name(name), dataLen(dataLen), data(data)
        {
        }

        void to(std::string& buff) const
        {
            size_t offset = buff.size();
            const size_t requiredSize = buff.size() + sizeof(int) + nameLen + 1 + sizeof(int) + dataLen + 1;
            buff.resize(requiredSize);

            std::memcpy(&buff[offset], &nameLen, sizeof(int));
            offset += sizeof(int);

            std::memcpy(&buff[offset], name, nameLen + 1);
            offset += nameLen + 1;

            std::memcpy(&buff[offset], &dataLen, sizeof(int));
            offset += sizeof(int);

            std::memcpy(&buff[offset], data, dataLen);
        }

        void from(char* file, int& offset)
        {
            std::memcpy(&nameLen, &file[offset], sizeof(int));
            offset += sizeof(int);

            name = &file[offset];
            offset += nameLen + 1;

            std::memcpy(&dataLen, &file[offset], sizeof(int));
            offset += sizeof(int);

            data = &file[offset];
            offset += dataLen + 1;
        }
    };

    struct PackHeader final
    {
        const char* header;
        int totalSize;
        int entries;

        void to(std::string& buff) const
        {
            const size_t requiredSize = HEADER_LEN + sizeof(int) * 2;
            if (buff.size() < requiredSize)
            {
                buff.resize(requiredSize);
            }

            size_t offset = 0;
            std::memcpy(&buff[offset], header, HEADER_LEN);
            offset += HEADER_LEN;

            std::memcpy(&buff[offset], &totalSize, sizeof(int));
            offset += sizeof(int);

            std::memcpy(&buff[offset], &entries, sizeof(int));
        }

        void from(char* file, int& offset)
        {
            offset = 0;
            header = &file[offset];
            offset += HEADER_LEN;

            std::memcpy(&totalSize, &file[offset], sizeof(int));
            offset += sizeof(int);

            std::memcpy(&entries, &file[offset], sizeof(int));
            offset += sizeof(int);
        }
    };

    static void UnCompressData(std::string& data)
    {
        const auto* start = data.data() + HEADER_LEN;
        const auto newData = new char[MAX_DECOMPRESSION_SIZE * 1024 * 1024];
        int newSize = sinflate(newData, MAX_DECOMPRESSION_SIZE * 1024 * 1024, start, data.size() - HEADER_LEN);

        data.resize(newSize + HEADER_LEN);
        std::memcpy(data.data() + HEADER_LEN, newData, newSize);
        delete[] newData;
    }

    static void CompressData(std::string& data)
    {
        int maxLen = sdefl_bound(data.size() - HEADER_LEN);
        const auto newData = new char[maxLen];

        auto* ctx = new sdefl();
        int newSize = sdeflate(ctx, newData, data.data() + HEADER_LEN, data.size() - HEADER_LEN, 5);

        data.resize(newSize + HEADER_LEN);
        std::memcpy(data.data() + HEADER_LEN, newData, newSize);

        delete[] newData;
        delete ctx;
    }

    static bool ParseImage(AssetPack& container, const uint64_t key)
    {
        auto& data = container.nativeData;
        if (!(data.starts_with(IMAGE_HEADER) || data.starts_with(IMAGE_HEADER_COMPRESSED)))
        {
            LOG_ERROR("Malformed asset pack file");
            return false;
        }

        SymmetricEncrypt(data.data() + HEADER_LEN, data.size() - HEADER_LEN, key);

        if (data.starts_with(IMAGE_HEADER_COMPRESSED))
        {
            UnCompressData(data);
        }

        int offset = 0;
        PackHeader header{};
        header.from(data.data(), offset);
        if ((int)data.size() != header.totalSize)
        {
            LOG_ERROR("AssetPack size mismatch between header and buffer size");
            return false;
        }

        container.assets.reserve(header.entries + 1);
        while (offset < (int)data.size())
        {
            PackEntry entry{};
            entry.from(data.data(), offset);
            if (entry.nameLen > 1000)
            {
                LOG_ERROR("Error parsing asset pack");
                return false;
            }
            container.assets.push_back({{entry.name, (size_t)entry.nameLen}, {entry.data, (size_t)entry.dataLen}});
        }
        container.sort();
        return true;
    }

    bool AssetPackLoad(AssetPack& assets, const char* path, const uint64_t key)
    {
        if (!fs::exists(path)) // User cant use AssetPack -> its empty
        {
            LOG_WARNING("No asset pack at: %s", path);
            return false;
        }

        const auto startTime = GetTime();
        if (!ReadFile(path, assets.nativeData))
        {
            LOG_ERROR("Failed to open asset pack file: %s", path);
            return false;
        }
        const int originalSize = assets.nativeData.size();
        const bool res = ParseImage(assets, key);
        const int currentSize = assets.nativeData.size();

        if (!res)
        {
            LOG_ERROR("Failed to parse asset pack: %s", path);
            return false;
        }

        const auto time = static_cast<int>(std::round((GetTime() - startTime) * 1000.0F)); // Round to millis
        if (originalSize == currentSize)
        {
            auto* logText = "Loaded asset pack %s | Took: %d millis | Total Size: %.2f mb | Assets: %d";
            LOG_INFO(logText, path, time, originalSize / 1'000'000.0F, assets.getSize());
        }
        else
        {
            auto* logText = "Loaded asset pack %s | Took: %d millis. Decompressed: %.2f mb -> "
                            "%.2f mb | Assets: %d";
            LOG_INFO(logText, path, time, originalSize / 1'000'000.0F, currentSize / 1'000'000.0F, assets.getSize());
        }

        return true;
    }

    static std::pair<int, int> GeneratePack(std::string& data, const PathList& pathList, std::string_view dir,
                                            bool compress, const uint64_t key)
    {
        fs::path root{dir};
        PackHeader header{IMAGE_HEADER, 1, 1};
        header.to(data);
        std::string relativePathStr;

        for (const auto& entry : pathList)
        {
            std::string fileData;
            if (!ReadFile(entry.generic_string(), fileData))
            {
                LOG_ERROR("Failed to open file: %s", entry.generic_string().c_str());
                continue;
            }
            relativePathStr = fs::relative(entry, root).generic_string();
            PackEntry asset{relativePathStr.c_str(), fileData.data(), (int)fileData.size()};
            asset.to(data);
        }

        header = {compress ? IMAGE_HEADER_COMPRESSED : IMAGE_HEADER, (int)data.size(), (int)pathList.size()};
        header.to(data);

        int originalSize = data.size();
        int compressedSize = 0;
        if (compress)
        {
            CompressData(data);
            compressedSize = data.size();
        }

        SymmetricEncrypt(data.data() + HEADER_LEN, data.size() - HEADER_LEN, key);
        return {originalSize, compressedSize};
    }

    static bool HasImageChanged(std::string_view packName, std::string_view newData)
    {
        std::string packData;
        if (!ReadFile(packName, packData))
        {
            return true;
        }
        if (newData.size() != packData.size())
        {
            return true;
        }
        return std::memcmp(packData.data(), newData.data(), packData.size()) != 0;
    }

    bool AssetPackCompile(const char* dir, const char* name, const uint64_t key, const bool compress)
    {
        const auto startTime = GetTime();

        PathList pathList;
        if (!CreatePathList(dir, pathList))
        {
            return false;
        }

        if (pathList.empty())
        {
            LOG_ERROR("No files to compile into asset pack");
            return true;
        }

        std::string data;
        auto [original, compressed] = GeneratePack(data, pathList, dir, compress, key);

        if (!HasImageChanged(name, data))
        {
            LOG_INFO("Skipped compiling asset pack: No changes detected");
            return true;
        }

        if (!WriteFile(name, data))
        {
            LOG_ERROR("Could not open file for writing: %s", name);
            return false;
        }

        const auto time = static_cast<int>(std::round((GetTime() - startTime) * 1000.0F)); // Round to millis
        if (compress)
        {
            auto* logText = "Compiled %s into %s | Took %d millis | Compressed: %.2f mb -> %.2f mb "
                            "(%.0f%%) | Assets: %d";
            LOG_INFO(logText, dir, name, time, original / 1'000'000.0F, compressed / 1'000'000.0F,
                     100.0F - (float)compressed / original * 100.0F, pathList.size());
        }
        else
        {
            auto* logText = "Compiled %s into %s | Took %d millis | Total Size: %.2f mb | Assets: %d";
            LOG_INFO(logText, dir, name, time, original / 1'000'000.0F, pathList.size());
        }
        return true;
    }

    // https://en.wikipedia.org/wiki/MD5#Pseudocode
    Checksum AssetPackChecksum(const char* path)
    {
        MAGIQUE_ASSERT(path != nullptr, "Passed null");
        Checksum checksum{};
        if (!fs::exists(path))
        {
            LOG_WARNING("No asset pack at:%s/%s", GetWorkingDirectory(), path);
            return checksum;
        }

        checksum.first = 0x67452301;
        checksum.second = 0xefcdab89;
        checksum.third = 0x98badcfe;
        checksum.fourth = 0x10325476;

        auto processChunk = [](uint32_t (&M)[16], Checksum& checksum)
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


        FILE* file = std::fopen(path, "rb");
        uint32_t chunk[16]{};

        std::fseek(file, 0, SEEK_END);
        const auto fileSize = static_cast<uint64_t>(ftell(file));
        std::fseek(file, 0, SEEK_SET);
        uint64_t totalBytesRead = 0;
        uint64_t bytesRead = 0;

        while (true)
        {
            bytesRead = std::fread(chunk, 1, 64, file);
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
            std::memset(byteChunk + bytesRead, 0, 64 - bytesRead);
            processChunk(chunk, checksum);

            std::memset(chunk, 0, 64);
            std::memcpy(byteChunk + 56, &bitLength, 8);
            processChunk(chunk, checksum);
        }
        else // Enough space to append the length
        {
            std::memset(byteChunk + bytesRead, 0, 56 - bytesRead);
            std::memcpy(byteChunk + 56, &bitLength, 8);
            processChunk(chunk, checksum);
        }

        fclose(file);
        return checksum;
    }

    bool AssetPackValidate(const Checksum checksum, const char* path)
    {
        MAGIQUE_ASSERT(checksum != Checksum{}, "Passed empty checksum");
        return AssetPackChecksum(path) == checksum;
    }

} // namespace magique
