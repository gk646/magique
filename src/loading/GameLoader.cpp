#include <magique/assets/AssetPacker.h>
#include <magique/loading/GameLoader.h>
#include <magique/util/Logging.h>

namespace magique
{

    GameLoader::GameLoader(const char* assetPath, uint64_t encryptionKey)
    {
        assets::LoadAssetImage(assetPath, assets, encryptionKey);
    }

    void GameLoader::printStats()
    {
        LOG_INFO("Registered %d tasks with a load pensum of: %d", gpuTasks.size() + cpuTasks.size(), totalImpact);
    }


    bool GameLoader::load() { return loadLoop(assets); }

    void GameLoader::registerTask(ITask<AssetContainer>* task, Thread thread, PriorityLevel pl, int impact)
    {

        if (task == nullptr)
        {
            LOG_WARNING("Tried to register task with nullptr");
            return;
        }

        if (impact < 0)
        {
            LOG_WARNING("Tried to register task with negative impact");
            return;
        }

        if (pl < 0 || pl > INSTANT)
        {
            LOG_WARNING("Tried to register task with invalid priority");
            return;
        }

        addTask(task, pl, thread, impact);
    }


    void GameLoader::registerTask(GameLoadFunc func, Thread thread, PriorityLevel pl, int impact)
    {
        if (func == nullptr)
        {
            LOG_WARNING("Tried to register task with nullptr");
            return;
        }

        if (impact < 0)
        {
            LOG_WARNING("Tried to register task with negative impact");
            return;
        }

        if (pl < 0 || pl > INSTANT)
        {
            LOG_WARNING("Tried to register task with invalid priority");
            return;
        }

        addLambdaTask(func, pl, thread, impact);
    }


} // namespace magique