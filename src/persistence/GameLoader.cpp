// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/persistence/GameLoader.h>

namespace magique
{
    GameLoader::GameLoader(const bool mainOnly) : mainOnly(mainOnly) {}

    bool GameLoader::registerTask(ITask<GameSave>* task, const PriorityLevel pl, const int impact)
    {
        return addTask(task, pl, mainOnly ? MAIN_THREAD : BACKGROUND_THREAD, impact);
    }

    bool GameLoader::registerTask(const GameLoadFunc& func, const PriorityLevel pl, const int impact)
    {
        return addLambdaTask(func, pl, mainOnly ? MAIN_THREAD : BACKGROUND_THREAD, impact);
    }

    void GameLoader::load(GameSave& save)
    {
        // Load as long as there are tasks
        while (!stepMixed(save))
            ;

        reset(); // Reset the loader so it can be reused
    }
    bool GameLoader::step() { return true; }

} // namespace magique