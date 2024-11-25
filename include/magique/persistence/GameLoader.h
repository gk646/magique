// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_GAMELOADER_H
#define MAGIQUE_GAMELOADER_H

#include <magique/internal/TaskExecutor.h>
#include <magique/persistence/container/GameSave.h>

//===============================================
// Game Loader
//===============================================
// ................................................................................
// This interface is meant as a helpful abstraction to structure potentially complex loading tasks.
// It is intended that create a single GameLoader where you define and register all tasks needed to load once.
// Like this its possible to reuse the loader by passing in loaded GameSaves (e.g. from LoadFromDisk(save, path)
// It will call all your loading task that extract data from the save and initialize your gameplay systems
// This makes it very trivial to manage different saves as each is loaded with the same stored routine
//
// Note: see assets/AssetLoader.h for a detailed description (same interface)
// Note: If 'mainOnly' is NOT specified, tasks can run on any available thread
// ................................................................................

namespace magique
{
    // For simple tasks not requiring variables
    // Called with the gamesave
    using GameLoadFunc = std::function<void(GameSave& save)>;

    struct GameLoader final : internal::TaskExecutor<GameSave>
    {
        // Creates an empty gameloader
        //      - mainOnly: if true only uses the main thread to execute the tasks
        explicit GameLoader(bool mainOnly = false);

        // Returns true if the loading task was successfully registered
        // task     - a new instance of a subclass of ITask, takes ownership
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        bool registerTask(ITask<GameSave>* task, PriorityLevel pl = MEDIUM, int impact = 1);

        // Returns true if the loading task was successfully registered
        // func     - a loading func (lambda)
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        bool registerTask(const GameLoadFunc& func, PriorityLevel pl = MEDIUM, int impact = 1);

        // Has to be called after all tasks are registered - this call blocks until all tasks are finished
        //      - save:     the save to load from
        void load(GameSave& save);

    private:
        bool step() override;
        bool mainOnly = false;
    };

} // namespace magique


#endif //MAGIQUE_GAMELOADER_H