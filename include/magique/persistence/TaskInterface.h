// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_TASK_INTERFACE_H
#define MAGIQUE_TASK_INTERFACE_H

#include <magique/internal/TaskExecutor.h>
#include <magique/persistence/GameSaveData.h>

//===============================================
// Task Interface
//===============================================
// ................................................................................
// This interface is meant as a helpful abstraction to structure complex persistence tasks.
// It allows you to describe complex loading/saving work in independent and stateless tasks that operate only on the given data.
// It is intended that this interface is created once (for each purpose) and initialized with the tasks.
// Then you simple invoke() the interface and all tasks are called with the specified threading and priority behavior
//
// Note: see assets/AssetLoader.h for a detailed description of the interface guarantees
// Note: see examples/demos/persistence to see potential usage
// Note: If 'mainOnly' is NOT specified, tasks can run on any available thread
// ................................................................................

namespace magique
{
    // For simple tasks not requiring variables
    // Called with the gamesave
    using GameLoadFunc = std::function<void(GameSaveData& save)>;

    struct TaskInterface final : internal::TaskExecutor<GameSaveData>
    {
        // Creates an empty gameloader
        //      - mainOnly: if true only uses the main thread to execute the tasks
        explicit TaskInterface(bool mainOnly = false);

        // Returns true if the loading task was successfully registered
        // task     - a new instance of a subclass of ITask, takes ownership
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        bool registerTask(ITask<GameSaveData>* task, PriorityLevel pl = MEDIUM, int impact = 1);

        // Returns true if the loading task was successfully registered
        // func     - a loading func (lambda)
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        bool registerTask(const GameLoadFunc& func, PriorityLevel pl = MEDIUM, int impact = 1);

        // Has to be called after all tasks are registered - this call blocks until all tasks are finished
        //      - save:     the save to load from
        void invoke(GameSaveData& save, const char* name);

    private:
        bool step() override;
        bool mainOnly = false;
    };

} // namespace magique


#endif //MAGIQUE_TASK_INTERFACE_H