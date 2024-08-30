#ifndef MAGIQUE_ASSETLOADER_H
#define MAGIQUE_ASSETLOADER_H

#include <magique/internal/TaskExecutor.h>
#include <magique/assets/container/AssetContainer.h>

//-----------------------------------------------
// Asset Loader
//-----------------------------------------------
// ................................................................................
// The main loader to get from startup to the initialized game (MainMenu)
// There are 2 guarantees when using the loader:
//  - The task is guaranteed be executed on the main thread if specified
//  - All task of a higher priority are finished before any task with a lower priority
//  - However there's no guarantee about the order for tasks of the same priority
//
// For ANY kind of gpu access (texture loading) you HAVE to specify MAIN_THREAD.
// For most others task use BACKGROUND_THREAD to allow background loading without stopping the render loop
// Loading dependencies and order can easily be created by specifying a lower priority for tasks that need data from previous tasks
// Note: This loader cleans itself up after loading all tasks and takes ownership of pointers passed
// .....................................................................

namespace magique
{
    using AssetLoadFunc = std::function<void(AssetContainer& assets)>; // For simple tasks not requiring variables

    struct AssetLoader final : internal::TaskExecutor<AssetContainer>
    {
        // Registers a new task - this is for more complex task requiring its own class -> subclass magique::ITask{};
        // task     - a new instance of a subclass of ITask, takes ownership
        // thread   - the thread where the task is loaded - ALL GPU ACCESS NEEDS TO HAPPEN ON THE MAIN THREAD (texture loading...)
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        void registerTask(ITask<AssetContainer>* task, ThreadType thread, PriorityLevel pl = MEDIUM, int impact = 1);

        // Registers a new task from a simple loading function - for smaller and less complex loading
        // func     - a loading func (lambda)
        // thread   - the thread where the task is loaded - ALL GPU ACCESS NEEDS TO HAPPEN ON THE MAIN THREAD (texture loading...)
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        // Example: registerTask([](magique::AssetContainer &assets) {}, magique::MAIN_THREAD);
        void registerTask(const AssetLoadFunc& func, ThreadType thread, PriorityLevel pl = MEDIUM, int impact = 1);

    private:
        AssetLoader(const char* assetPath, uint64_t encryptionKey);
        bool step() override;
        AssetContainer assets;
        friend Game;
    };

} // namespace magique

#endif //MAGIQUE_ASSETLOADER_H