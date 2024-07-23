#ifndef JOBSCHEDULER_CUH
#define JOBSCHEDULER_CUH

#include <deque>
#include <atomic>
#include <condition_variable>
#include <vector>

#include "internal/headers/OSUtil.h"

namespace magique
{
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

    struct Scheduler final
    {
        ~Scheduler()
        {
            shutDown = true;
            isHibernate = true;
            for (auto& t : threads)
            {
                if (t.joinable())
                    t.join();
            }
        }

        alignas(64) Spinlock queueLock;                    // The lock to make queue access thread safe
        alignas(64) Spinlock workedLock;                   // The lock to worked vector thread safe
        alignas(64) std::deque<IJob*> jobQueue;            // Global job queue
        alignas(64) std::atomic<bool> isHibernate = false; // If the scheduler is running
        alignas(64) std::vector<const IJob*> workedJobs;   // Currently processed jobs
        alignas(64) std::atomic<bool> shutDown = false;    // Signal to shutdown all threads
        std::atomic<int> currentJobsSize = 0;              // Current jobs
        std::atomic<uint16_t> handleID = 0;                // The internal handle counter
        std::thread::id mainID;                            // Thread id of the main thread
        std::vector<std::thread> threads;                  // All working threads
        std::atomic<int> usingThreads = 0;

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

        jobHandle getNextHandle()
        {
            return static_cast<jobHandle>(handleID++);
        }

        friend void WorkerThreadFunc(Scheduler* scheduler, int threadNumber);
    };

    inline void WorkerThreadFunc(Scheduler* scheduler, const int threadNumber)
    {
        SetupThreadPriority(threadNumber, false);
        while (!scheduler->shutDown.load(std::memory_order_acquire))
        {
            while (!scheduler->isHibernate.load(std::memory_order_acquire))
            {
                scheduler->queueLock.lock();
                if (!scheduler->jobQueue.empty())
                {
                    // printf("Taken by: %d\n", GetCurrentThreadId());
                    const auto job = scheduler->jobQueue.front();
                    scheduler->jobQueue.pop_front();
                    scheduler->queueLock.unlock();
                    M_ASSERT(job->handle != jobHandle::null, "Null handle");
                    //cxstructs::now(1);
                    job->run();
                    //cxstructs::printTime<std::chrono::nanoseconds>("Took:", 1);
                    scheduler->removeWorkedJob(job);
                    delete job;
                }
                else
                {
                    scheduler->queueLock.unlock();
                }
                std::this_thread::yield();
            }
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }


    namespace global
    {
        inline Scheduler SCHEDULER{};
    }
} // namespace magique

#endif //JOBSCHEDULER_CUH