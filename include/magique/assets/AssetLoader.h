#ifndef MAGIQUE_ASSETLOADER_H
#define MAGIQUE_ASSETLOADER_H

#include <magique/assets/container/AssetContainer.h>
#include <magique/internal/TaskExecutor.h>

//-----------------------------------------------
// Game Loader
//-----------------------------------------------
// ................................................................................
// The main loader to get from startup to the initialized game (MainMenu)
// Theres 2 guarantees when using the loader:
//  - The task is guaranteed be executed on the main thread if specified
//  - All task of a higher priority are finished when advancing to the next
//
// For ANY kind of gpu access (texture loading) you HAVE specify MAIN_THREAD.
// For most others task use ANY_THREAD to allow background loading without stopping the render loop
// Loading dependencies and order can easily be created by specifying a lower priority for tasks that accesses data
// of higher ones
// This loader cleans itself up after loading all tasks

namespace magique
{
    using AssetLoadFunc = void (*)(AssetContainer&); // Typedef for simple tasks not requiring variables

    struct AssetLoader final : TaskExecutor<AssetContainer>
    {
        AssetLoader(const char* assetPath, uint64_t encryptionKey);

        // Registers a new task
        // task     - a new instance of a subclass of ITask, takes owner ship
        // thread   - the thread where the task is loaded - ALL GPU ACCESS NEEDS TO HAPPEN ON THE MAIN THREAD (texture loading...)
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        void registerTask(TaskI<AssetContainer>* task, Thread thread, PriorityLevel pl = MED, int impact = 1);

        // Registers a new task
        // func     - a loading func (lambda)
        // thread   - the thread where the task is loaded - ALL GPU ACCESS NEEDS TO HAPPEN ON THE MAIN THREAD (texture loading...)
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        void registerTask(AssetLoadFunc func, Thread thread, PriorityLevel pl = MED, int impact = 1);

    private:
        // Prints current stats - automatically called if all task are registered
        void printStats() const;

        // Called each frame - progressed the loader
        bool step() override;

        AssetContainer assets;
        friend Game;
    };

} // namespace magique

#endif //MAGIQUE_ASSETLOADER_H