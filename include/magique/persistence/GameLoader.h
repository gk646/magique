#ifndef MAGIQUE_GAMELOADER_H
#define MAGIQUE_GAMELOADER_H

#include <magique/fwd.hpp>
#include <magique/util/TaskExecutor.h>

//-----------------------------------------------
// Game Saver
//-----------------------------------------------
// ................................................................................
//
// ................................................................................
namespace magique
{
    using GameSaveFunc = void (*)(SaveGame&); // Typedef for simple tasks not requiring variables

    struct SaveSaver final : TaskExecutor<AssetContainer>
    {
        GameLoader(const char* assetPath, uint64_t encryptionKey);

        // Prints current stats - automatically called if all task are registered
        void printStats() const;

        // Called each frame - progressed the loader
        // Dont call it yourself
        bool save() override;

        // Registers a new task
        // task     - a new instance of a subclass of ITask, takes owner ship
        // thread   - the thread where the task is loaded - ALL GPU ACCESS NEEDS TO HAPPEN ON THE MAIN THREAD (texture loading...)
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        void registerTask(ITask<AssetContainer>* task, Thread thread, PriorityLevel pl = MED, int impact = 1);

        // Registers a new task
        // func     - a loading func (lambda)
        // thread   - the thread where the task is loaded - ALL GPU ACCESS NEEDS TO HAPPEN ON THE MAIN THREAD (texture loading...)
        // pl       - the level of priority, higher priorities are loaded first
        // impact   - an absolute estimate of the time needed to finish the task
        void registerTask(GameLoadFunc func, Thread thread, PriorityLevel pl = MED, int impact = 1);

    private:
        SaveGame saveGame;
    };

} // namespace magique


#endif //MAGIQUE_GAMELOADER_H