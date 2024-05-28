#ifndef TASKEXECUTOR_H_
#define TASKEXECUTOR_H_

#include <thread>
#include <map>
#include <vector>
#include <cstdint>

enum PriorityLevel : uint8_t
{
    INSTANT,
    CRITICAL,
    HIGH,
    MED,
    LOW,
    DONE
};
enum Device : uint8_t
{
    CPU,
    GPU
};

template <typename T>
struct ITask
{
    bool isLoaded = false;
    int impact = 0;
    virtual void execute(T& res) = 0;
};

struct IExecutor
{
    virtual ~IExecutor() = default;
    virtual auto load() -> bool = 0;
    [[nodiscard]] virtual auto getProgressPercent() const -> float = 0;
    [[nodiscard]] virtual auto isFinished() const -> bool = 0;
};

template <typename T>
struct TaskExecutor : IExecutor
{
    ~TaskExecutor() override
    {
        if (loadThread.joinable())
        {
            loadThread.join();
        }
        for (auto& loadLevel : cpuTasks)
        {
            for (auto task : loadLevel.second)
                delete task;
        }
        for (auto& loadLevel : gpuTasks)
        {
            for (auto task : loadLevel.second)
                delete task;
        }
    }
    auto getProgressPercent() const -> float final { return (100.0F * loadedImpact) / totalImpact; }
    auto isFinished() const -> bool final { return currentLevel == DONE; }

protected:
    void registerTask(ITask<T>* ptr, PriorityLevel pl, const Device d, int impact) {
        if (!ptr) {
            printf("MAGE_QUEST: Total Load Pensum: %d\n", totalImpact);
            return;
        }
        if (d == GPU) {
            gpuTasks[pl].push_back(ptr);
        } else {
            cpuTasks[pl].push_back(ptr);
        }
        ptr->impact = impact;
        totalImpact += impact;
    }
    bool loadLoop(T& res)
    {
        if (currentLevel == DONE)
            return true;

        if (currentLevel == INSTANT)
        {
            loadInstant(res);
        }

        // Load the next GPU task if not currently loading a GPU task and if there are any left to load
        if (!gpuDone)
        {
            bool done = true;
            if (!gpuTasks[currentLevel].empty())
            {
                for (auto task : gpuTasks[currentLevel])
                {
                    if (!task->isLoaded)
                    {
                        loadTask(task, res);
                        done = false;
                        break;
                    }
                }
            }
            else
            {
                done = true;
            }
            gpuDone = done;
        }

        if (!cpuDone)
        {
            if (!cpuTasks[currentLevel].empty())
            {
                if (loadThread.joinable())
                {
                    loadThread.join();
                }
                loadThread = std::thread(
                    [&]()
                    {
                        for (auto& task : cpuTasks[currentLevel])
                        {
                            if (!task->isLoaded)
                            {
                                loadTask(task, res);
                            }
                        }
                        cpuDone = true;
                    });
            }
            else
            {
                cpuDone = true;
            }
        }

        // Check if it's time to move to the next level
        if (gpuDone && cpuDone)
        {
            bool allTasksLoaded = true;
            for (const auto& task : cpuTasks[currentLevel])
            {
                if (!task->isLoaded)
                {
                    allTasksLoaded = false;
                    break;
                }
            }

            if (allTasksLoaded)
            {
                for (const auto& task : gpuTasks[currentLevel])
                {
                    if (!task->isLoaded)
                    {
                        allTasksLoaded = false;
                        break;
                    }
                }
            }

            if (allTasksLoaded)
            {
                gpuDone = false;
                cpuDone = false;
                currentLevel = static_cast<PriorityLevel>(static_cast<int>(currentLevel) + 1);
            }
        }
        return isFinished();
    }
    void loadInstant(T& res)
    {
        for (auto task : gpuTasks[currentLevel])
        {
            if (!task->isLoaded)
            {
                loadTask(task, res);
            }
        }
    }
    void loadTask(ITask<T>* ptr, T& res)
    {
        ptr->execute(res);
        loadedImpact += ptr->impact;
        ptr->isLoaded = true;
        printf("MAGE_QUEST: %d / %d -> %f completed\n", loadedImpact.load(), totalImpact, getProgressPercent());
    }
    std::map<PriorityLevel, std::vector<ITask<T>*>> cpuTasks;
    std::map<PriorityLevel, std::vector<ITask<T>*>> gpuTasks;
    std::thread loadThread;
    int totalImpact = 0;
    std::atomic<int> loadedImpact = 0;
    PriorityLevel currentLevel = INSTANT;
    bool cpuDone = false;
    bool gpuDone = false;
};

#endif //MAGEQUEST_SRC_UTILS_TASKEXECUTOR_H_