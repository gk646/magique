#ifndef MAGIQUE_JOBS_H
#define MAGIQUE_JOBS_H

#include <condition_variable>
#include <deque>
#include <vector>
#include <thread>
#include <magique/fwd.hpp>

//-----------------------------------------------
// Job System
//-----------------------------------------------
// .....................................................................
// This is for advanced users
// Use GetScheduler() to get the global instance and schedule your tasks
// This systems is trimmed for speed - many while true loops during tick
//
// .....................................................................

namespace magique
{
    // handle to a job
    enum class jobHandle : uint16_t
    {
        null = UINT16_MAX, // The null handle
    };

    // Job base class - allows to call templatd lambdas
    struct IJob
    {
        // No virtual destructor to save instructions
        virtual void run() = 0;
        jobHandle handle = jobHandle::null;
    };

    // Wrapper for a spinlock
    struct Spinlock final
    {
        void lock()
        {
            while (flag.test_and_set(std::memory_order_acquire))
                ;
        }

        void unlock() { flag.clear(std::memory_order_release); }

    private:
        std::atomic_flag flag = ATOMIC_FLAG_INIT;
    };

    // Core scheduler
    // Takes ownership of all pointers
    struct Scheduler final
    {
        explicit Scheduler(int threadCount = 4);
        ~Scheduler();

        // Adds a new job to the global queue
        jobHandle addJob(IJob* job);

        // Adds the job to a group
        jobHandle groupJob(IJob* job, int group);

        // dependencies
        // accumulate

        // Awaits the completion of all handles in the given iterable container
        template <typename Container>
        void await(const Container& handles) const
        {
            while (currentJobsSize > 0)
            {
                bool allCompleted = true;
                const auto vec = workedJobs.data();
                for (const auto handle : handles)
                {
                    for (int i = 0; i < currentJobsSize; ++i)
                    {
                        if (vec[i]->handle == handle)
                        {
                            allCompleted = false;
                            break;
                        }
                    }
                    if (!allCompleted)
                        break;
                }
                if (allCompleted)
                    return;
            }
        }

        // Awaits the completion of all current tasks
        void awaitAll() const;

        //----------------- internal -----------------//
        // These function are public to make it a complete module
        // You dont need to call them when using magique

        // Brings all workers back to speed
        void wakeup();

        // Puts all workers to hibernation
        void hibernate();

    private:
        void addWorkedJob(const IJob* job)
        {
            // allows for time tracking later on
            workedLock.lock();
            ++currentJobsSize;
            workedJobs.push_back(job);
            workedLock.unlock();
        }
        void removeWorkedJob(const IJob* job)
        {
            // allows for time tracking later on
            workedLock.lock();
            --currentJobsSize;
            std::erase(workedJobs, job);
            workedLock.unlock();
            // Just spin the handles around
            if (handleID >= 65000)
            {
                handleID = 0;
            }
        }

        jobHandle getNextHandle() { return static_cast<jobHandle>(handleID++); }
        friend void workerThread(Scheduler* scheduler);

        // Aligned to prevent false sharing
        alignas(64) Spinlock queueLock;                    // The lock to make queue access thread safe
        alignas(64) Spinlock workedLock;                   // The lock to worked vector thread safe
        alignas(64) std::deque<IJob*> jobQueue;            // Global job queue
        alignas(64) std::atomic<bool> isHibernate = false; // If the scheduler is running
        alignas(64) std::vector<const IJob*> workedJobs;   // Currently processed jobs
        alignas(64) std::atomic<bool> shutDown = false;    // Signal to shutdown all threads
        alignas(64) std::condition_variable condition;
        volatile std::atomic<int> currentJobsSize = 0;
        std::atomic<uint16_t> handleID = 0; // The internal handle counter
        std::thread::id mainID;             // Thread id of the main thread
        std::vector<std::thread> threads;   // All working threads
        std::atomic<int> usingThreads = 0;
    };


    template <typename Callable>
    struct Job final : IJob
    {
        explicit Job(Callable func) : func_(std::move(func)) {}
        void run() override { func_(); }

    private:
        Callable func_;
    };

    // Allows to specify explicitly specify parameters
    template <typename Func, typename... Args>
    struct ExplicitJob final : IJob
    {
        explicit ExplicitJob(Func func, Args... args) : func(std::move(func)), args(std::make_tuple(args...)) {}

        void run() override { std::apply(func, args); }

    private:
        Func func;
        std::tuple<Args...> args;
    };

    struct Worker
    {
        std::deque<IJob*> jobQueue;
        std::thread thread;
    };

} // namespace magique

#endif //MAGIQUE_JOBS_H