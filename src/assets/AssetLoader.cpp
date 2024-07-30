#include <magique/assets/AssetPacker.h>
#include <magique/assets/AssetLoader.h>
#include <magique/util/Logging.h>

namespace magique
{
    AssetLoader::AssetLoader(const char* assetPath, uint64_t encryptionKey)
    {
        LoadAssetImage(assetPath, assets, encryptionKey);
    }

    void AssetLoader::printStats() const
    {
        LOG_INFO("Registered %d tasks with a load pensum of: %d", gpuTasks.size() + cpuTasks.size(), totalImpact);
    }

    bool AssetLoader::step() { return stepLoop(assets); }

    template <typename Func>
    bool BasicChecks(const Func func, const PriorityLevel pl, const int impact)
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

        if (pl < 0 || pl > INSTANT)
        {
            LOG_WARNING("Tried to register task with invalid priority");
            return false;
        }
        return true;
    }

    void AssetLoader::registerTask(TaskI<AssetContainer>* task, const ThreadType thread, const PriorityLevel pl,
                                   const int impact)
    {
        if (!BasicChecks(task, pl, impact))
            return;
        addTask(task, pl, thread, impact);
    }

    void AssetLoader::registerTask(AssetLoadFunc func, const ThreadType thread, const PriorityLevel pl, const int impact)
    {
        if (!BasicChecks(func, pl, impact))
            return;
        addLambdaTask(func, pl, thread, impact);
    }
} // namespace magique