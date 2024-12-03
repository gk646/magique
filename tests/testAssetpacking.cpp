// SPDX-License-Identifier: zlib-acknowledgement

#define CATCH_CONFIG_MAIN // Include somewhere once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <catch_amalgamated.hpp>
#include <magique/assets/AssetLoader.h>
#include <magique/assets/AssetPacker.h>
#include <raylib/raylib.h>

constexpr auto DATA_PATH = "test-data.bin";
constexpr auto INDEX = "index.magique";

using namespace magique;

static void testImpl(const uint64_t key, const bool compress)
{
    namespace fs = std::filesystem;
    CompileAssetImage("./", DATA_PATH, key, compress);

    AssetContainer container;
    LoadAssetImage(container, DATA_PATH, key); // Compression detected automatically

    const auto& assets = container.getAllAssets();

    for (const auto& entry : fs::directory_iterator("./"))
    {
        if (!entry.is_regular_file() || entry.path().filename() == "test-data.bin" ||
            entry.path().filename() == "index.magique")
        {
            continue;
        }

        std::string relativePath = entry.path().relative_path();
        relativePath.erase(relativePath.begin());
        relativePath.erase(relativePath.begin());
        const int fileSize = static_cast<int>(fs::file_size(entry.path()));

        // Read the file's content
        std::ifstream file(entry.path(), std::ios::binary);
        assert(file.is_open() && "Failed to open file");

        std::vector<char> fileData(fileSize);
        file.read(fileData.data(), fileSize);
        file.close();

        bool found = false;
        for (const auto& asset : assets)
        {
            if (relativePath == asset.path && fileSize == asset.size &&
                memcmp(fileData.data(), asset.data, fileSize) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::cerr << "File missing in assets: " << relativePath << '\n';
            remove(DATA_PATH);
            remove(INDEX);
            assert(false && "File not found in assets");
        }
    }

    remove(DATA_PATH);
    remove(INDEX);
}

TEST_CASE("default") { testImpl(0, false); }

TEST_CASE("encryption") { testImpl(GetRandomValue(0, INT32_MAX), false); }

TEST_CASE("encryption+compression") { testImpl(GetRandomValue(0, INT32_MAX), true); }