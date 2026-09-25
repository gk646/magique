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
#include <magique/util/Data.h>
#include <magique/internal/glaze/core/common.hpp>
#include <magique/internal/glaze/beve/read.hpp>
#include <magique/internal/glaze/beve/write.hpp>

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
                if (!DataReadFile(entry.path().generic_string(), fileData))
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
        if (!DataReadFile(packName, packData))
        {
            return true;
        }
        if (newData.size() != packData.size())
        {
            return true;
        }
        return std::memcmp(packData.data(), newData.data(), packData.size()) != 0;
    }

    bool AssetPackCompile(std::string_view dir, std::string_view name, EncryptionKey key)
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
            LOG_ERROR("Failed to generate asset pack: %s", glz::format_error(error).data());
            return false;
        }

        int originalSize = data.size();
        data = std::move(internal::DataCompressImpl(data));
        int compressedSize = data.size();
        DataEncrypt(data, key);

        if (!HasImageChanged(name, data))
        {
            LOG_INFO("Skipped writing new asset pack: No changes detected");
            return true;
        }

        if (!DataWriteFile(name, data))
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

    bool AssetPackLoad(AssetPack& pack, std::string_view path, EncryptionKey key)
    {
        if (!fs::exists(path))
        {
            LOG_WARNING("No asset pack at: %s", path.data());
            return false;
        }

        const auto startTime = GetTime();
        if (!DataReadFile(path, pack.nativeData))
        {
            LOG_ERROR("Failed to open asset pack file: %s", path.data());
            return false;
        }

        DataDecrypt(pack.nativeData, key);

        const int originalSize = pack.nativeData.size();
        auto result = internal::DataDecompressImpl(pack.nativeData);
        if (!result)
        {
            LOG_WARNING("Failed to decompress asset pack: %s", path.data());
            return false;
        }
        pack.nativeData = std::move(result.value().get());
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

    Hash AssetPackChecksum(std::string_view path)
    {
        Hash checksum{};
        if (!fs::exists(path))
        {
            LOG_WARNING("No asset pack at:%s/%s", GetWorkingDirectory(), path);
            return checksum;
        }

        std::string data;
        if (!DataReadFile(path, data))
            return checksum;

        return DataHash(data);
    }

    bool AssetPackValidate(const Hash checksum, std::string_view path)
    {
        MAGIQUE_ASSERT(checksum != Hash{}, "Passed empty checksum");
        return AssetPackChecksum(path) == checksum;
    }

} // namespace magique
