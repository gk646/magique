// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSETLOADER_H
#define MAGIQUE_ASSETLOADER_H

#include <magique/internal/TaskExecutor.h>
#include <magique/assets/container/AssetContainer.h>

//===============================================
// Asset Loader
//===============================================
// ................................................................................
// Allows to register tasks that operate on all assets loaded from disk
// You can register your task at Game::onStartup() and it runs automatically after this method
//
// The loader gives 2 strong guarantees:
//  - Order:
//         - All task of a higher priority are finished before any task with a lower priority
//  - Threading:
//         - ThreadType == MAIN_THREAD -> task runs on the main thread
//         - ThreadType == BACKGROUND_THREAD -> task runs on ANY available thread
//
// This means higher priority is loaded first -> If a task depends on another one give it a lower priority
// IMPORTANT: For ANY kind of gpu access (texture loading) you HAVE to specify MAIN_THREAD.
//            For most others task use BACKGROUND_THREAD to allow background loading without stopping the render loop
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

        // Registers a simple loading function - for smaller and less complex loading
        // func     - a loading func (lambda)
        // thread   - the thread where the task is loaded - ALL GPU ACCESS NEEDS TO HAPPEN ON THE MAIN THREAD (texture loading...)
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        // Example: registerTask([](magique::AssetContainer &assets) {}, magique::MAIN_THREAD);
        void registerTask(const AssetLoadFunc& func, ThreadType thread, PriorityLevel pl = MEDIUM, int impact = 1);

    private:
        PUB(AssetLoader(const char* assetPath, uint64_t encryptionKey));
        bool step() override;
        AssetContainer assets;
        friend Game;
    };

} // namespace magique

#endif //MAGIQUE_ASSETLOADER_H