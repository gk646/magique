#ifndef MAGIQUE_GAMELOADER_H
#define MAGIQUE_GAMELOADER_H

#include <magique/core/Types.h>
#include <magique/internal/TaskExecutor.h>
#include <magique/persistence/container/GameSave.h>

//===============================================
// Game Saver
//===============================================
// ................................................................................
// This interface is meant as a helpful abstraction to the user.
// You should create it once as a global variable and then reuse it (e.g. call it when the user wants to save the game)
//
// Note: Uses the same interface as magique::AssetLoader but all tasks automatically run on a background thread
// ................................................................................

namespace magique
{
    // For simple tasks not requiring variables
    // Called with the gamesave
    using GameLoadFunc = std::function<void(GameSave& save)>;

    struct GameLoader final : internal::TaskExecutor<GameSave>
    {

        // Registers a new loading task
        // task     - a new instance of a subclass of ITask, takes ownership
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        void registerTask(ITask<GameSave>* task, PriorityLevel pl = MEDIUM, int impact = 1);

        // Registers a new loading function
        // func     - a loading func (lambda)
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        void registerTask(const GameLoadFunc& func, PriorityLevel pl = MEDIUM, int impact = 1);

    private:
        bool step() override;
        GameSave gameSave;
    };

} // namespace magique


#endif //MAGIQUE_GAMELOADER_H