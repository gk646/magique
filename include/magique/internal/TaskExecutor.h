// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_TASKEXECUTOR_H_
#define MAGIQUE_TASKEXECUTOR_H_

#include <functional>
#include <vector>
#include <atomic>

#include <magique/util/Logging.h>
#include <magique/core/Types.h>
#include <magique/util/JobSystem.h>


//===============================================
// Task Executor
//===============================================
// ................................................................................
// Generic Execution interface with strong guarantees about task priorities and background thread support
//
// ................................................................................

namespace magique::internal
{
    template <typename T>
    struct LambdaTask final : ITask<T>
    {
        std::function<void(T&)> func;
        explicit LambdaTask(std::function<void(T&)> func) : func(std::move(func)) {}
        void execute(T& res) override { func(res); }
    };

    struct IExecutor
    {
        virtual ~IExecutor() = default;
        virtual bool step() = 0;
        [[nodiscard]] virtual float getProgressPercent() const = 0;
        [[nodiscard]] virtual bool isStartup() const = 0;
    };

    template <typename Func>
    bool IsTaskValid(const Func& func, const PriorityLevel pl, const int impact)
    {
        if (func == nullptr)
        {
            LOG_WARNING("Tried to register task with nullptr");
            return false;
        }
        if (impact < 0)
        {
            LOG_WARNING("Tried to register task with negative impact");
            return false;
        }

        if (pl < 0 || pl > CRITICAL)
        {
            LOG_WARNING("Tried to register task with invalid priority");
            return false;
        }
        return true;
    }

    template <typename T>
    struct TaskExecutor : IExecutor
    {

        ~TaskExecutor() override
        {
            for (auto& vec : cpuTasks)
            {
                for (auto task : vec)
                    delete task;
            }
            for (auto& vec : gpuTasks)
            {
                for (auto task : vec)
                    delete task;
            }
        }

        [[nodiscard]] float getProgressPercent() const final { return 100.0F * loadedImpact / totalImpact; }

    protected:
        [[nodiscard]] bool isStartup() const final { return std::is_same_v<T, AssetContainer>; }

        void printStats() const
        {
            LOG_INFO("Registered %d tasks with a load pensum of: %d", getTotalTasks(), totalImpact);
        }

        [[nodiscard]] int getTotalTasks() const
        {
            int tasks = 0;
            for (int i = 0; i < CRITICAL; ++i)
            {
                tasks += static_cast<int>(gpuTasks[i].size());
            }
            for (int i = 0; i < CRITICAL; ++i)
            {
                tasks += static_cast<int>(cpuTasks[i].size());
            }
            return tasks;
        }

        bool stepMixed(T& res)
        {
            if (currentLevel == -1)
            {
                return true;
            }

            if (currentLevel == INTERNAL && !gpuTasks[currentLevel].empty())
            {
                loadTasks(gpuTasks[currentLevel], res);
            }

            if (!gpuDone)
            {
                gpuDone = loadTasks(gpuTasks[currentLevel], res);
            }

            if (!cpuDone && !cpuWorking)
            {
                if (!cpuTasks[currentLevel].empty())
                {
                    cpuWorking = true;
                    AddJob(CreateJob(
                        [this, &res]
                        {
                            loadTasks(cpuTasks[currentLevel], res);
                            cpuDone = true;
                        }));
                }
                else
                {
                    cpuDone = true;
                }
            }

            if (gpuDone && cpuDone)
            {
                if (isVectorLoaded(cpuTasks[currentLevel]) && isVectorLoaded(gpuTasks[currentLevel]))
                {
                    gpuDone = false;
                    cpuDone = false;
                    cpuWorking = false;
                    currentLevel = static_cast<PriorityLevel>(static_cast<int>(currentLevel) - 1);
                }
            }

            return currentLevel == -1;
        }

        bool addTask(ITask<T>* task, PriorityLevel pl, const ThreadType d, int impact, bool internal = false)
        {
            if (!internal && !IsTaskValid(task, pl, impact))
            {
                return false;
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
            return true;
        }

        bool addLambdaTask(std::function<void(T&)> func, const PriorityLevel pl, const ThreadType d, const int impact,
                           bool internal = false)
        {
            addTask(new LambdaTask{func}, pl, d, impact, internal);
            return true;
        }

        void reset()
        {
            for (auto& vec : cpuTasks)
            {
                for (auto task : vec)
                {
                    task->isLoaded = false;
                }
            }
            for (auto& vec : gpuTasks)
            {
                for (auto task : vec)
                {
                    task->isLoaded = false;
                }
            }
            totalImpact = 0;
            loadedImpact = 0;
            currentLevel = INTERNAL;
            cpuDone = false;
            gpuDone = false;
            cpuWorking = false;
        }

        std::vector<ITask<T>*> cpuTasks[INTERNAL + 1]{};
        std::vector<ITask<T>*> gpuTasks[INTERNAL + 1]{};
        int totalImpact = 0;
        std::atomic<int> loadedImpact = 0;
        PriorityLevel currentLevel = INTERNAL;
        bool cpuDone = false;
        bool gpuDone = false;
        bool cpuWorking = false;

    private:
        bool loadTasks(std::vector<ITask<T>*>& tasks, T& res)
        {
            for (auto task : tasks)
            {
                if (!task->getIsLoaded())
                {
                    loadTask(task, res);
                }
            }
            return true;
        }

        void loadTask(ITask<T>* task, T& res)
        {
            task->execute(res);
            loadedImpact += task->impact;
            task->isLoaded = true;
            if (currentLevel == INTERNAL)
            {
                return;
            }
            LOG_INFO("Loaded Task: Impact: %d | Progress: %d/%d -> %.2f%%", task->impact, loadedImpact.load(),
                     totalImpact, getProgressPercent());
        }

        bool isVectorLoaded(const std::vector<ITask<T>*>& tasks) const
        {
            for (const auto task : tasks)
            {
                if (!task->getIsLoaded())
                {
                    return false;
                }
            }
            return true;
        }

        friend struct ITask<T>;
    };
} // namespace magique::internal
#endif //MAGIQUE_TASKEXECUTOR_H_