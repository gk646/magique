#ifndef MAGIQUE_JOBS_H
#define MAGIQUE_JOBS_H

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
// .....................................................................

namespace magique
{
    enum JobType : uint8_t
    {
        VECTOR, // Vector operation
    };
    // handle to a job
    enum class jobHandle : uint16_t
    {
        null = UINT16_MAX, // The null handle
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

    struct JobBase
    {
        // No virtual destructor
        virtual void run() = 0;
        jobHandle handle;
    };

    template <typename Callable>
    struct Job final : JobBase
    {
        explicit Job(Callable func) : func_(std::move(func)) {}

        void run() override { func_(); }

    private:
        Callable func_;
    };

    template <typename Func, typename... Args>
    struct ExplicitJob final : JobBase
    {
        explicit ExplicitJob(Func func, Args... args) : func_(func), args_(std::make_tuple(args...)) {}

        void run() override { std::apply(func_, args_); }

    private:
        Func func_;
        std::tuple<Args...> args_;
    };


    template <typename T>
    using VectorOperation = void (*)(const T* data, uint32_t index);

    template <typename T>
    struct VectorJob final : JobBase
    {
        VectorJob(VectorOperation<T> op, T* ptr, const uint32_t begin, const uint32_t end) :
            op(op), ptr(ptr), begin(begin), end(end)
        {
        }

        void run() override
        {
            for (uint32_t i = begin; i < end; ++i)
            {
                op(ptr, i);
            }
        }

        VectorOperation<T> op;
        const T* ptr;
        uint32_t begin;
        uint32_t end;
    };


    // Core scheduler
    struct Scheduler final
    {
        explicit Scheduler(int threadCount = 4);
        ~Scheduler();

        // Adds a job that operates on a single data vector
        template <typename T>
        jobHandle addVectorJob(VectorOperation<T> op, T* data, uint32_t begin, uint32_t end)
        {
            const auto handle = getNextHandle();
            JobBase* job = new VectorJob<T>(op, data, begin, end);
            job->handle = handle;
            queueLock.lock();
            jobQueue.push_back(job);
            queueLock.unlock();
            addWorkedJob(job);
            return handle;
        }

        // Adds a generic job with explicit type parameters
        template <typename Func, typename... Args>
        jobHandle addJobEx(Func func, Args... args)
        {
            const auto handle = getNextHandle();
            JobBase* job = new ExplicitJob<Func, Args...>{func, std::forward<Args>(args)...};
            job->handle = handle;
            queueLock.lock();
            jobQueue.push_back(job);
            queueLock.unlock();
            addWorkedJob(job);
            return handle;
        }

        // Adds a  job with explicit type parameters
        template <typename Func>
        jobHandle addJob(Func func)
        {
            const auto handle = getNextHandle();
            JobBase* job = new Job<Func>{func};
            job->handle = handle;
            queueLock.lock();
            jobQueue.push_back(job);
            queueLock.unlock();
            addWorkedJob(job);
            return handle;
        }

        // dependencies
        // accumulate

        // Starts all threads
        void start();
        // Stops all threads
        void stop();

        void await(const std::initializer_list<jobHandle>& handles);

    private:
        void addWorkedJob(const JobBase* job)
        {
            // allows for time tracking later on
            workedLock.lock();
            workedJobs.push_back(job);
            workedLock.unlock();
        }

        void removeWorkedJob(const JobBase* job)
        {
            // allows for time tracking later on
            workedLock.lock();
            std::erase(workedJobs,job);
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
        alignas(64) Spinlock queueLock;                 // The lock to make queue access thread safe
        alignas(64) Spinlock workedLock;                // The lock to worked vector thread safe
        alignas(64) std::atomic<uint16_t> handleID = 0; // The internal handle counter
        alignas(64) std::atomic<bool> running = false;  // If the scheduler is running
        alignas(64) std::deque<JobBase*> jobQueue;      // Global job queue
        std::thread::id mainID;                         // Thread id of the main thread
        std::vector<std::thread> threads;               // All working threads
        std::vector<const JobBase*> workedJobs;         // Currently processed jobs
        std::atomic<bool> shutDown = false;             // Signal to shutdown all threads
    };

} // namespace magique

#endif //MAGIQUE_JOBS_H