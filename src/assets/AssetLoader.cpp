#include <magique/assets/AssetPacker.h>
#include <magique/assets/AssetLoader.h>
#include <magique/util/Logging.h>

namespace magique
{
    AssetLoader::AssetLoader(const char* assetPath, const uint64_t encryptionKey)
    {

        addLambdaTask([=](AssetContainer& assets) { LoadAssetImage( assets,assetPath, encryptionKey); }, INTERNAL,
                      BACKGROUND_THREAD, 0, true);
    }

    bool AssetLoader::step() { return stepMixed(assets); }

    bool BasicChecks(const void* func, const PriorityLevel pl, const int impact)
    {
        if (func == nullptr)
        {
            LOG_WARNING("Tried to register task with nullptr");
            return false;
        }
        if (impact < 0)
        {
            LOG_WARNING("Tried to register task with negative impact");
            return false;
        }

        if (pl < 0 || pl > CRITICAL)
        {
            LOG_WARNING("Tried to register task with invalid priority");
            return false;
        }
        return true;
    }

    void AssetLoader::registerTask(ITask<AssetContainer>* task, const ThreadType thread, const PriorityLevel pl,
                                   const int impact)
    {
        if (!BasicChecks(task, pl, impact))
            return;
        addTask(task, pl, thread, impact);
    }

    void AssetLoader::registerTask(const AssetLoadFunc& func, const ThreadType thread, const PriorityLevel pl,
                                   const int impact)
    {
        if (!BasicChecks(&func, pl, impact))
            return;
        addLambdaTask(func, pl, thread, impact);
    }
} // namespace magique