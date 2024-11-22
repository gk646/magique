#ifndef MAGIQUE_GAMESAVER_H
#define MAGIQUE_GAMESAVER_H

#include <magique/internal/TaskExecutor.h>
#include <magique/persistence/container/GameSave.h>

//===============================================
// Game Saver
//===============================================
// ................................................................................
// This interface is meant as a helpful abstraction to structure potentially complex saving tasks.
// You only need to define and register all tasks needed to save once and can then reuse them.
// You then pass an empty GameSave into the save and all your tasks should add all the data needed to persist the gamestate
// Once done you can persist the GameSave to disk with: SaveToDisk(save,path);
// This makes it very trivial to manage different saves as each is loaded with the same stored routine
//
// Note: see assets/AssetLoader.h for a detailed description (same interface)
// Note: If 'mainOnly' is NOT specified, tasks can run on any available thread
// ................................................................................

namespace magique
{
    // For simple tasks not requiring variables
    // Called with the gamesave
    using GameSaveFunc = std::function<void(GameSave& save)>;

    struct GameSaver final : internal::TaskExecutor<GameSave>
    {
        // Creates an empty gamesaver
        //      - mainOnly: if true only uses the main thread to execute the tasks
        explicit GameSaver(bool mainOnly = false);

        // Returns true if the loading task was successfully registered
        // task     - a new instance of a subclass of ITask, takes ownership
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        bool registerTask(ITask<GameSave>* task, PriorityLevel pl = MEDIUM, int impact = 1);

        // Returns true if the loading task was successfully registered
        // func     - a loading func (lambda)
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        bool registerTask(const GameSaveFunc& func, PriorityLevel pl = MEDIUM, int impact = 1);

        // Has to be called after all tasks are registered - this call blocks until all tasks are finished
        //      - save:     the save to load from
        void save(GameSave& save);

    private:
        bool step() override;
        bool mainOnly = false;
    };

} // namespace magique


#endif //MAGIQUE_GAMESAVER_H