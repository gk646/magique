#ifndef TASKEXECUTOR_H_
#define TASKEXECUTOR_H_

#include <magique/util/Logging.h>
#include <thread>
#include <map>
#include <vector>
#include <cstdint>
#include <functional>

namespace magique
{
    enum PriorityLevel : int8_t
    {
        LOW,
        MED,
        MEDIUM,
        CRITICAL,
        INSTANT,
    };

    enum Thread : uint8_t
    {
        MAIN_THREAD,
        BACKGROUND_THREAD,
    };

    template <typename T>
    struct TaskI
    {
        bool isLoaded = false;
        int impact = 0;

        virtual ~TaskI() = default;
        virtual void execute(T& res) = 0;
    };

    template <typename T>
    struct LambdaTask final : TaskI<T>
    {
        std::function<void(T&)> func;

        explicit LambdaTask(std::function<void(T&)> func) : func(std::move(func)) {}
        void execute(T& res) override { func(res); }
    };

    struct ExecutorI
    {
        virtual ~ExecutorI() = default;
        virtual bool load() = 0;
        [[nodiscard]] virtual float getProgressPercent() const = 0;
    };

    template <typename T>
    struct TaskExecutor : ExecutorI
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
        float getProgressPercent() const final { return 100.0F * loadedImpact / totalImpact; }

    protected:
        void addTask(TaskI<T>* task, PriorityLevel pl, const Thread d, int impact)
        {
            if (!task)
            {
                return;
            }
            task->impact = impact;
            totalImpact += impact;
            if (d == MAIN_THREAD)
            {
                gpuTasks[pl].push_back(task);
            }
            else
            {
                cpuTasks[pl].push_back(task);
            }
        }
        void addLambdaTask(std::function<void(T&)> func, const PriorityLevel pl, const Thread d, const int impact)
        {
            addTask(new LambdaTask{func}, pl, d, impact);
        }
        bool loadLoop(T& res)
        {
            if (currentLevel == -1)
            {
                return true;
            }

            if (currentLevel == INSTANT && gpuTasks.contains(currentLevel))
            {
                loadTasks(gpuTasks[currentLevel], res);
            }

            if (!gpuDone)
            {
                gpuDone = loadTasks(gpuTasks[currentLevel], res);
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
                        [this, &res]()
                        {
                            loadTasks(cpuTasks[currentLevel], res);
                            cpuDone = true;
                        });
                }
                else
                {
                    cpuDone = true;
                }
            }

            if (gpuDone && cpuDone)
            {
                bool allTasksLoaded =
                    areAllTasksLoaded(cpuTasks[currentLevel]) && areAllTasksLoaded(gpuTasks[currentLevel]);
                if (allTasksLoaded)
                {
                    gpuDone = false;
                    cpuDone = false;
                    currentLevel = static_cast<PriorityLevel>(static_cast<int>(currentLevel) - 1);
                }
            }

            return currentLevel == -1;
        }
        bool loadTasks(std::vector<TaskI<T>*>& tasks, T& res)
        {
            for (auto task : tasks)
            {
                if (!task->isLoaded)
                {
                    loadTask(task, res);
                }
            }
            return true;
        }
        void loadTask(TaskI<T>* task, T& res)
        {
            task->execute(res);
            loadedImpact += task->impact;
            task->isLoaded = true;
            LOG_INFO("Loaded Task: Impact: %d | Progress: %d/%d -> %.2f%%", task->impact, loadedImpact.load(),
                     totalImpact, getProgressPercent());
        }
        bool areAllTasksLoaded(const std::vector<TaskI<T>*>& tasks) const
        {
            for (const auto task : tasks)
            {
                if (!task->isLoaded)
                {
                    return false;
                }
            }
            return true;
        }

        std::map<PriorityLevel, std::vector<TaskI<T>*>> cpuTasks;
        std::map<PriorityLevel, std::vector<TaskI<T>*>> gpuTasks;
        std::thread loadThread;
        int totalImpact = 0;
        std::atomic<int> loadedImpact = 0;
        PriorityLevel currentLevel = INSTANT;
        bool cpuDone = false;
        bool gpuDone = false;
    };
} // namespace magique
#endif //TASKEXECUTOR_H_