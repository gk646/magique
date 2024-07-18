#ifndef MAGIQUE_GAMELOADER_H
#define MAGIQUE_GAMELOADER_H

#include <magique/core/Types.h>
#include <magique/internal/TaskExecutor.h>
#include <magique/persistence/container/GameSave.h>

//-----------------------------------------------
// Game Saver
//-----------------------------------------------
// ................................................................................
// All tasks automatically run on a background thread
// ................................................................................
namespace magique
{
    using GameLoadFunc = void (*)(GameSave&); // Typedef for simple tasks not requiring variables

    struct GameLoader final : TaskExecutor<GameSave>
    {
        GameLoader(const char* assetPath, uint64_t encryptionKey);

        // Registers a new task
        // task     - a new instance of a subclass of ITask, takes owner ship
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        void registerTask(TaskI<GameSave>* task, PriorityLevel pl = MEDIUM, int impact = 1);

        // Registers a new task
        // func     - a loading func (lambda)
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        void registerTask(GameLoadFunc func, PriorityLevel pl = MEDIUM, int impact = 1);

    private:
        // Prints current stats - automatically called if all task are registered
        void printStats() const;

        // Called each frame - progresses the loader
        bool step() override;

        GameSave gameSave;
        friend Game;
    };

} // namespace magique


#endif //MAGIQUE_GAMELOADER_H