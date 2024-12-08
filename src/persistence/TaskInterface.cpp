// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/persistence/TaskInterface.h>

namespace magique
{
    TaskInterface::TaskInterface(const bool mainOnly) : mainOnly(mainOnly) {}

    bool TaskInterface::registerTask(ITask<GameSaveData>* task, const PriorityLevel pl, const int impact)
    {
        return addTask(task, pl, mainOnly ? THREAD_MAIN : THREAD_ANY, impact);
    }

    bool TaskInterface::registerTask(const GameLoadFunc& func, const PriorityLevel pl, const int impact)
    {
        return addLambdaTask(func, pl, mainOnly ? THREAD_MAIN : THREAD_ANY, impact);
    }

    void TaskInterface::invoke(GameSaveData& save)
    {
        if (totalTasks == 0)
        {
            LOG_WARNING("Trying to load with an empty interface");
        }

        // Load as long as there are tasks
        while (!stepMixed(save))
            ;

        reset(); // Reset the loader so it can be reused
    }

    bool TaskInterface::step() { return true; }

} // namespace magique