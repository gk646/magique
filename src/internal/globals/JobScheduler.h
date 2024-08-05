#ifndef JOBSCHEDULER_H
#define JOBSCHEDULER_H

#include <deque>
#include <thread>

#include <magique/internal/Macros.h>

#include "external/raylib/src/external/glfw/include/GLFW/glfw3.h"
#include "internal/headers/OSUtil.h"
#include "internal/types/Spinlock.h"
#include "internal/datastructures/VectorType.h"

namespace magique
{
    struct Scheduler final
    {
        alignas(64) Spinlock queueLock;                    // The lock to make queue access thread safe
        alignas(64) Spinlock workedLock;                   // The lock to worked vector thread safe
        alignas(64) std::atomic<bool> isHibernate = false; // If the scheduler is running
        alignas(64) std::atomic<bool> shutDown = false;    // Signal to shutdown all threads
        alignas(64) std::deque<IJob*> jobQueue;            // Global job queue
        alignas(64) vector<const IJob*> workedJobs;        // Currently processed jobs
        std::atomic<int> currentJobsSize = 0;              // Current jobs
        std::atomic<uint16_t> handleID = 0;                // The internal handle counter
        vector<std::thread> threads;                       // All working threads
        std::atomic<int> usingThreads = 0;
        double targetTime = 0;
        double sleepTime = 0;

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
            workedJobs.erase_unordered(job);
            workedLock.unlock();
            // Just spin the handles around
            if (handleID >= 65000)
            {
                handleID = 0;
            }
        }

        void close()
        {
            shutDown = true;
            isHibernate = true;
            for (auto& t : threads)
            {
                if (t.joinable())
                    t.join();
            }
            threads.clear();
        }

        jobHandle getNextHandle() { return static_cast<jobHandle>(handleID++); }

        friend void WorkerThreadFunc(Scheduler* scheduler, int threadNumber);
    };

    inline void WorkerThreadFunc(Scheduler* scheduler, const int threadNumber)
    {
        SetupThreadPriority(threadNumber);
        bool waited = false;
        while (!scheduler->shutDown.load(std::memory_order_acquire))
        {
            while (!scheduler->isHibernate.load(std::memory_order_acquire))
            {
                waited = false;
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
            }

            // Wait more precisely with spinning at the end - only if necessary
            if (!waited)
            {
                if (glfwGetTime() + scheduler->sleepTime < scheduler->targetTime)
                    WaitTime(scheduler->targetTime, scheduler->sleepTime);
                waited = true;
            }
        }
    }

    namespace global
    {
        inline Scheduler SCHEDULER{};
    }
} // namespace magique

#endif //JOBSCHEDULER_CUH