#include <magique/assets/AssetPacker.h>
#include <magique/core/GameLoader.h>
#include <magique/util/Logging.h>

namespace magique
{

    GameLoader::GameLoader(const char* assetPath, uint64_t encryptionKey)
    {
        LoadAssetImage(assetPath, std::move(assets), encryptionKey);
    }

    void GameLoader::printStats() const
    {
        LOG_INFO("Registered %d tasks with a load pensum of: %d", gpuTasks.size() + cpuTasks.size(), totalImpact);
    }

    bool GameLoader::load()
    {
        return loadLoop(assets);
    }

    bool BasicChecks(void* func, PriorityLevel pl, int impact)
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

    void GameLoader::registerTask(ITask<AssetContainer>* task, Thread thread, PriorityLevel pl, int impact)
    {
        if (!BasicChecks(task, pl, impact))
            return;

        addTask(task, pl, thread, impact);
    }


    void GameLoader::registerTask(GameLoadFunc func, Thread thread, PriorityLevel pl, int impact)
    {
        if (!BasicChecks(func, pl, impact))
            return;

        addLambdaTask(func, pl, thread, impact);
    }


} // namespace magique