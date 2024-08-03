#ifndef MAGIQUE_TASKEXECUTOR_H_
#define MAGIQUE_TASKEXECUTOR_H_

#include <functional>
#include <vector>
#include <atomic>

#include <magique/util/Logging.h>
#include <magique/core/Types.h>
#include <magique/util/Jobs.h>

namespace magique
{
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
        virtual bool step() = 0;
        [[nodiscard]] virtual float getProgressPercent() const = 0;
    };

    template <typename T>
    struct TaskExecutor : ExecutorI
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
        float getProgressPercent() const final { return 100.0F * loadedImpact / totalImpact; }

    protected:
        void printStats() const
        {
            LOG_INFO("Registered %d tasks with a load pensum of: %d", getTotalTasks(), totalImpact);
        }
        int getTotalTasks() const
        {
            int tasks = 0;
            for (const auto& vec : gpuTasks)
            {
                tasks += vec.size();
            }
            for (const auto& vec : cpuTasks)
            {
                tasks += vec.size();
            }
            return tasks;
        }
        bool stepLoop(T& res)
        {
            if (currentLevel == -1)
            {
                return true;
            }

            if (currentLevel == INSTANT && !gpuTasks[currentLevel].empty())
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
                bool allTasksLoaded =
                    areAllTasksLoaded(cpuTasks[currentLevel]) && areAllTasksLoaded(gpuTasks[currentLevel]);
                if (allTasksLoaded)
                {
                    gpuDone = false;
                    cpuDone = false;
                    cpuWorking = false;
                    currentLevel = static_cast<PriorityLevel>(static_cast<int>(currentLevel) - 1);
                }
            }

            return currentLevel == -1;
        }
        void addTask(TaskI<T>* task, PriorityLevel pl, const ThreadType d, int impact)
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
        void addLambdaTask(std::function<void(T&)> func, const PriorityLevel pl, const ThreadType d, const int impact)
        {
            addTask(new LambdaTask{func}, pl, d, impact);
        }
        std::vector<TaskI<T>*> cpuTasks[INSTANT + 1]{};
        std::vector<TaskI<T>*> gpuTasks[INSTANT + 1]{};
        int totalImpact = 0;
        std::atomic<int> loadedImpact = 0;
        PriorityLevel currentLevel = INSTANT;
        bool cpuDone = false;
        bool gpuDone = false;
        bool cpuWorking = false;

    private:
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
    };
} // namespace magique
#endif //MAGIQUE_TASKEXECUTOR_H_