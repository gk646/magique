// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/persistence/GameSaver.h>


namespace magique
{

    GameSaver::GameSaver(const bool mainOnly) : mainOnly(mainOnly) {}

    bool GameSaver::registerTask(ITask<GameSave>* task, const PriorityLevel pl, const int impact)
    {
        return addTask(task, pl, mainOnly ? MAIN_THREAD : BACKGROUND_THREAD, impact);
    }

    bool GameSaver::registerTask(const GameSaveFunc& func, const PriorityLevel pl, const int impact)
    {
        return addLambdaTask(func, pl, mainOnly ? MAIN_THREAD : BACKGROUND_THREAD, impact);
    }

    void GameSaver::save(GameSave& save)
    {
        // Load as long as there are tasks
        while (!stepMixed(save))
            ;
        reset();
    }

    bool GameSaver::step() { return true; }

} // namespace magique