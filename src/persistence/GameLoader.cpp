#include <magique/persistence/GameLoader.h>

namespace magique
{
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

        if (pl < 0 || pl > INTERNAL)
        {
            LOG_WARNING("Tried to register task with invalid priority");
            return false;
        }
        return true;
    }

    bool GameLoader::step() { return stepLoop(gameSave); }

    void GameLoader::registerTask(TaskI<GameSave>* task, PriorityLevel pl, int impact)
    {
        if (!BasicChecks(task, pl, impact))
            return;

        addTask(task, pl, BACKGROUND_THREAD, impact);
    }

    void GameLoader::registerTask(GameLoadFunc func, PriorityLevel pl, int impact)
    {
        if (!BasicChecks(func, pl, impact))
            return;

        addLambdaTask(func, pl, BACKGROUND_THREAD, impact);
    }


} // namespace magique