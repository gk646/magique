// SPDX-License-Identifier: zlib-acknowledgement

#define CATCH_CONFIG_MAIN // Include somewhere once

#include <catch_amalgamated.hpp>

#include <magique/assets/AssetLoader.h>
#include <magique/assets/AssetPacker.h>

using namespace magique;

struct Setup final
{
    Setup() { CompileAssetImage("./"); }
};

static Setup SETUP{};

static AssetLoader LOADER{"./", 0};

static void test_impl(uint64_t key)
{

}


TEST_CASE("register Tasks")
{

    LOADER.registerTask()

}