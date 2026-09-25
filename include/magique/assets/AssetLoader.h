// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSETLOADER_H
#define MAGIQUE_ASSETLOADER_H

#include <magique/internal/TaskExecutor.h>
#include <magique/assets/AssetPack.h>

//===============================================
// AssetLoader
//===============================================
// ................................................................................
// Allows to register tasks that operate on all assets loaded from disk
// You can register your tasks inside Game::onStartup() and they are executed automatically after
//
// The loader gives 2 strong guarantees:
//  - Order:
//         - All task of a higher priority are finished before any task with a lower priority
//  - Threading:
//         - ThreadType == THREAD_MAIN -> task runs on the main thread
//         - ThreadType == THREAD_ANY  -> task runs on ANY available background thread (if configured MAGIQUE_WORKER_THREADS)
//
// This means higher priority is loaded first -> If a task depends on another one give it a lower priority
// IMPORTANT: For ANY kind of gpu access (texture loading) you HAVE to specify THREAD_MAIN.
//            For most others task use THREAD_ANY to allow background loading without stopping the render loop
// .....................................................................

namespace magique
{
    using AssetLoadFunc = std::function<void(AssetPack& assets)>; // For simple tasks not requiring variables

    struct AssetLoader final : internal::TaskExecutor<AssetPack>
    {
        // Registers a new task - this is for more complex task requiring its own class -> subclass magique::ITask{};
        // task     - a new instance of a subclass of ITask, takes ownership
        // thread   - thread where the task is loaded - GPU ACCESS NEEDS TO HAPPEN ON THE MAIN THREAD (texture loading...)
        // pl       - the level of priority, higher priorities are loaded first
        void registerTask(ITask<AssetPack>* task, ThreadType thread, PriorityLevel pl = MEDIUM);

        // Registers a simple loading function - for smaller and less complex loading
        // func     - a loading func (lambda)
        // thread   - thread where the task is loaded - GPU ACCESS NEEDS TO HAPPEN ON THE MAIN THREAD (texture loading...)
        // pl       - the level of priority, higher priorities are loaded first
        // Example: registerTask([](magique::AssetPack &assets) {}, magique::MAIN_THREAD);
        void registerTask(const AssetLoadFunc& func, ThreadType thread = THREAD_MAIN, PriorityLevel pl = MEDIUM);

    private:
        MQ_MAKE_PUB()
        AssetLoader(std::string_view assetPath, EncryptionKey key);
        bool step() override;
        AssetPack assets;
        friend Game;
    };

} // namespace magique

#endif //MAGIQUE_ASSETLOADER_H