// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_JOB_SCHEDULER_H
#define MAGIQUE_JOB_SCHEDULER_H

#include <deque>
#include <thread>
#include <raylib/raylib.h>

#include <magique/internal/Macros.h>

#include "internal/utils/OSUtil.h"
#include "internal/utils/STLUtil.h"
#include "internal/types/SpinLock.h"
#include "internal/datastructures/VectorType.h"
#include "external/cxstructs/cxallocator/SlotAllocator.h"

M_IGNORE_WARNING(4324) // structure was padded due to alignment specifier

namespace magique
{
    struct Scheduler final
    {
        alignas(64) std::deque<IJob*> jobQueue;     // Global job queue
        alignas(64) vector<const IJob*> workedJobs; // Currently processed jobs
        vector<std::thread> threads;                // All working threads
        cxstructs::SlotAllocator<50> jobAllocator;  // Allocator for jobs
        SpinLock queueLock;                         // The lock to make queue access thread safe
        SpinLock workedLock;                        // The lock to worked vector thread safe
        std::atomic<bool> shutDown = false;         // Signal to shut down all threads
        std::atomic<bool> isHibernate = false;      // If the scheduler is running
        std::atomic<int> currentJobsSize = 0;       // Current jobs
        std::atomic<uint16_t> handleID = 0;         // The internal handle counter
        double targetTime = 0;
        double sleepTime = 0;

        ~Scheduler() { close(); } // Added for safety

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
            jobAllocator.free(job);
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
            jobAllocator.destroy();
        }

        jobHandle getNextHandle()
        {
            return static_cast<jobHandle>(handleID++);
        }

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

M_UNIGNORE_WARNING()

#endif //MAGIQUE_JOB_SCHEDULER_H