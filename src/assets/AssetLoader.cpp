// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/assets/AssetPacker.h>
#include <magique/assets/AssetLoader.h>

namespace magique
{
    AssetLoader::AssetLoader(const char* assetPath, const uint64_t encryptionKey)
    {
        addLambdaTask([=](AssetContainer& assets) { LoadAssetImage(assets, assetPath, encryptionKey); }, INTERNAL,
                      THREAD_ANY, 0, true);
    }

    bool AssetLoader::step() { return stepMixed(assets); }


    void AssetLoader::registerTask(ITask<AssetContainer>* task, const ThreadType thread, const PriorityLevel pl,
                                   const int impact)
    {
        addTask(task, pl, thread, impact);
    }

    void AssetLoader::registerTask(const AssetLoadFunc& func, const ThreadType thread, const PriorityLevel pl,
                                   const int impact)
    {
        addLambdaTask(func, pl, thread, impact);
    }
} // namespace magique