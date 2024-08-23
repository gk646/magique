#ifndef MAGIQUE_JOBSCHEDULER_H
#define MAGIQUE_JOBSCHEDULER_H

#include <deque>
#include <thread>
#include <raylib/raylib.h>

#include <magique/internal/Macros.h>

#include "internal/utils/OSUtil.h"
#include "internal/utils/STLUtil.h"
#include "internal/types/Spinlock.h"
#include "internal/datastructures/VectorType.h"
#include "external/cxstructs/cxallocator/allocators/BumpAllocator.h"

#pragma warning(push)
#pragma warning(disable : 4324) // structure was padded due to alignment specifier

namespace magique
{
    struct Scheduler final
    {
        alignas(64) std::deque<IJob*> jobQueue;             // Global job queue
        alignas(64) vector<const IJob*> workedJobs;         // Currently processed jobs
        vector<std::thread> threads;                        // All working threads
        cxstructs::DynamicBumpAllocator jobAllocator{1000}; // Allocator for jobs
        Spinlock queueLock;                                 // The lock to make queue access thread safe
        Spinlock workedLock;                                // The lock to worked vector thread safe
        std::atomic<bool> shutDown = false;                 // Signal to shutdown all threads
        std::atomic<bool> isHibernate = false;              // If the scheduler is running
        std::atomic<int> currentJobsSize = 0;               // Current jobs
        std::atomic<uint16_t> handleID = 0;                 // The internal handle counter
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
            UnorderedDelete(workedJobs, job);
            if(workedJobs.empty())
                jobAllocator.reset();
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
                    const auto job = scheduler->jobQueue.front();
                    scheduler->jobQueue.pop_front();
                    scheduler->queueLock.unlock();
                    MAGIQUE_ASSERT(job->handle != jobHandle::null, "Null handle");
                    job->run();
                    scheduler->removeWorkedJob(job);
                }
                else
                {
                    scheduler->queueLock.unlock();
                }
            }

            // Wait more precisely with spinning at the end - only if necessary
            if (!waited)
            {
                if (GetTime() + scheduler->sleepTime < scheduler->targetTime)
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

#pragma warning(pop)

#endif //MAGIQUE_JOBSCHEDULER_H