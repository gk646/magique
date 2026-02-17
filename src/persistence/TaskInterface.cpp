// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/persistence/SaveLoader.h>

namespace magique
{
    SaveLoader::SaveLoader(const bool mainOnly) : mainOnly(mainOnly) {}

    bool SaveLoader::registerTask(ITask<GameSave>* task, const PriorityLevel pl, const int impact)
    {
        return addTask(task, pl, mainOnly ? THREAD_MAIN : THREAD_ANY, impact);
    }

    bool SaveLoader::registerTask(const GameLoadFunc& func, const PriorityLevel pl, const int impact)
    {
        return addLambdaTask(func, pl, mainOnly ? THREAD_MAIN : THREAD_ANY, impact);
    }

    void SaveLoader::invoke(GameSave& save, const char* name)
    {
        if (totalTasks == 0)
        {
            LOG_WARNING("Trying to load an empty interface: %s", name);
        }
        else
        {
            LOG_INFO("Loading interface: %s", name);
        }

        // Load as long as there are tasks
        while (!stepMixed(save))
        {
        }

        reset(); // Reset the loader so it can be reused
    }

    bool SaveLoader::step() { return true; }

} // namespace magique