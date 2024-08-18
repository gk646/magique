#ifndef MAGIQUE_GAMELOADER_H
#define MAGIQUE_GAMELOADER_H

#include <magique/core/Types.h>
#include <magique/internal/TaskExecutor.h>
#include <magique/persistence/container/GameSave.h>

//-----------------------------------------------
// Game Saver
//-----------------------------------------------
// ................................................................................
// Uses the same interface as magique::AssetLoader but all tasks automatically run on a background thread
// ................................................................................

namespace magique
{
    using GameLoadFunc = std::function<void(GameSave&)>; // For simple tasks not requiring variables

    struct GameLoader final : internal::TaskExecutor<GameSave>
    {

        // Registers a new task
        // task     - a new instance of a subclass of ITask, takes owner ship
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        void registerTask(ITask<GameSave>* task, PriorityLevel pl = MEDIUM, int impact = 1);

        // Registers a new task
        // func     - a loading func (lambda)
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        void registerTask(const GameLoadFunc& func, PriorityLevel pl = MEDIUM, int impact = 1);

    private:
        GameLoader(const char* assetPath, uint64_t encryptionKey);
        bool step() override;
        GameSave gameSave;
    };

} // namespace magique


#endif //MAGIQUE_GAMELOADER_H