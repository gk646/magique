// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <filesystem>
#include <vector>
#include <cstring>
#include <cmath>
#include <raylib/raylib.h>

#include <magique/assets/AssetPacker.h>
#include <magique/assets/AssetPack.h>
#include <magique/util/Logging.h>
#include <magique/util/RayUtils.h>
#include <magique/internal/glaze/core/common.hpp>
#include <magique/internal/glaze/beve/read.hpp>
#include <magique/internal/glaze/beve/write.hpp>

#include "internal/utils/EncryptionUtil.h"

namespace fs = std::filesystem;

namespace magique
{
    struct AssetPackHeader final
    {
        int totalSize;
        int entries;
    };

    struct AssetPackFile
    {
        std::string path;
        std::string data;
    };

    // Owns the data to write
    struct AssetPackWrite
    {
        AssetPackHeader header;
        std::vector<AssetPackFile> files;
    };

    // Zero copy view to read data into
    struct AssetPackView
    {
        AssetPackHeader header;
        std::vector<Asset> files;
    };

    static bool ReadFile(std::string_view name, std::string& data)
    {
        FILE* file = fopen(name.data(), "rb");
        if (file == nullptr)
        {
            LOG_INFO("Failed to open file: %s", name.data());
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

    static bool WriteFile(std::string_view fileName, std::string_view content)
    {
        FILE* file = fopen(fileName.data(), "w+b");
        if (file == nullptr)
        {
            LOG_ERROR("Could not open file for writing: %s", fileName);
            return false;
        }

        fwrite(content.data(), content.size(), 1, file);
        fclose(file);
        return true;
    }

    static void ScanDirectory(const fs::path& root, const fs::path& directory, AssetPackWrite& pack)
    {
        const auto iter = fs::directory_iterator(directory);
        for (const auto& entry : iter)
        {
            if (entry.is_directory())
            {
                ScanDirectory(root, entry.path(), pack);
            }
            else if (entry.is_regular_file())
            {
                std::string fileData;
                if (!ReadFile(entry.path().generic_string(), fileData))
                    continue;
                pack.header.totalSize += fileData.size();
                pack.header.entries++;
                auto fileName = fs::relative(entry, root).generic_string();
                pack.files.emplace_back(std::move(fileName), std::move(fileData));
            }
        }
    }

    static bool CreatePathList(std::string_view directory, AssetPackWrite& pack)
    {
        fs::path dirPath(directory);
        std::error_code ec;
        const fs::file_status status = fs::status(dirPath, ec);
        if (ec)
        {
            LOG_ERROR("Cannot access path: %s", directory.data());
            return false;
        }

        if (fs::is_directory(status))
        {
            pack.files.reserve(128);
            ScanDirectory(dirPath, dirPath, pack);
        }
        else
        {
            LOG_ERROR("Given path is not a directory: %s", directory.data());
            return false;
        }
        return true;
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

    bool AssetPackCompile(std::string_view dir, std::string_view name, const uint64_t key)
    {
        const auto startTime = GetTime();

        AssetPackWrite writePack;
        if (!CreatePathList(dir, writePack))
            return false;

        if (writePack.files.empty())
        {
            LOG_WARNING("No files given to compile asset pack");
            return true;
        }

        std::string data;
        auto error = glz::write_beve(writePack, data);
        if (error)
        {
            LOG_ERROR("Failed to generate assset pack: %s", glz::format_error(error).data());
            return false;
        }

        int originalSize = data.size();
        {
            auto [compressed, isCompressed] = CompressData(data);
            data = compressed;
            CompressData({}); // Clear compression buffer
            if (!isCompressed)
            {
                LOG_ERROR("Failed to compress asset pack");
                return false;
            }
        }
        int compressedSize = data.size();
        SymmetricEncrypt(data.data(), data.size(), key);

        if (!HasImageChanged(name, data))
        {
            LOG_INFO("Skipped writing new asset pack: No changes detected");
            return true;
        }

        if (!WriteFile(name, data))
        {
            LOG_ERROR("Could not open file for writing: %s", name);
            return false;
        }

        const auto time = static_cast<int>(std::round((GetTime() - startTime) * 1000.0F)); // Round to millis
        auto* fmt = "Compiled %s into %s | Took %d millis | Compressed: %.2f mb -> %.2f mb (%+.0f%%) | Assets: %d";
        LOG_INFO(fmt, dir.data(), name.data(), time, originalSize / 1'000'000.0F, compressedSize / 1'000'000.0F,
                (-1.0F + (float)compressedSize / originalSize) * 100.0F, writePack.files.size());
        return true;
    }

    bool AssetPackLoad(AssetPack& pack, std::string_view path, const uint64_t key)
    {
        if (!fs::exists(path))
        {
            LOG_WARNING("No asset pack at: %s", path.data());
            return false;
        }

        const auto startTime = GetTime();
        if (!ReadFile(path, pack.nativeData))
        {
            LOG_ERROR("Failed to open asset pack file: %s", path.data());
            return false;
        }

        SymmetricEncrypt(pack.nativeData.data(), pack.nativeData.size(), key);

        const int originalSize = pack.nativeData.size();
        pack.nativeData = DecompressData(pack.nativeData);
        DecompressData({}); // Clear compression buffer
        const int currentSize = pack.nativeData.size();

        {
            AssetPackView assetView{};
            const auto error = glz::read_beve(assetView, pack.nativeData);
            if (error)
            {
                LOG_ERROR("Failed to read asset pack: %s", glz::format_error(error).data());
                return false;
            }

            pack.assets = std::move(assetView.files);
            // Null-terminate the strings
            for (auto asset : pack.assets)
            {
                *((char*)asset.path.data() + asset.path.size()) = '\0';
                *((char*)asset.data.data() + asset.data.size()) = '\0';
            }

            pack.sort();
        }

        const auto time = static_cast<int>(std::round((GetTime() - startTime) * 1000.0F)); // Round to millis
        auto* fmt = "Loaded asset pack %s | Took: %d millis. Decompressed: %.2f mb -> %.2f mb | Assets: %d";
        LOG_INFO(fmt, path.data(), time, originalSize / 1'000'000.0F, currentSize / 1'000'000.0F, pack.getSize());
        return true;
    }

    // https://en.wikipedia.org/wiki/MD5#Pseudocode
    Checksum AssetPackChecksum(std::string_view path)
    {
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

        FILE* file = std::fopen(path.data(), "rb");
        uint32_t chunk[16]{};

        std::fseek(file, 0, SEEK_END);
        const auto fileSize = static_cast<uint64_t>(ftell(file));
        std::fseek(file, 0, SEEK_SET);
        uint64_t bytesRead = 0;

        while (true)
        {
            bytesRead = std::fread(chunk, 1, 64, file);

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
