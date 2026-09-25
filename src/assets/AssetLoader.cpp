// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/assets/AssetPacker.h>
#include <magique/assets/AssetLoader.h>

namespace magique
{
    void AssetLoader::registerTask(ITask<AssetPack>* task, const ThreadType thread, const PriorityLevel pl)
    {
        addTask(task, pl, thread, 1);
    }

    void AssetLoader::registerTask(const AssetLoadFunc& func, const ThreadType thread, const PriorityLevel pl)
    {
        addLambdaTask(func, pl, thread, 1);
    }

    AssetLoader::AssetLoader(std::string_view assetPath, const EncryptionKey key)
    {
        addLambdaTask([=](AssetPack& assets)
        {
            AssetPackLoad(assets, assetPath, key);
        }, INTERNAL, THREAD_ANY, 0, true);
    }

    bool AssetLoader::step() { return stepMixed(assets); }

} // namespace magique
