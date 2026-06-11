// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_JOB_SCHEDULER_H
#define MAGIQUE_JOB_SCHEDULER_H

#include <deque>
#include <thread>
#include <raylib/raylib.h>

#include "internal/utils/OSUtil.h"
#include "internal/types/SpinLock.h"
#include "external/cxstructs/cxallocator/SlotAllocator.h"

namespace magique
{
    struct JobData final
    {
        alignas(64) std::deque<IJob*> queuedJobs;        // Global job queue
        alignas(64) std::vector<const IJob*> workedJobs; // Currently processed jobs
        std::vector<std::thread> threads;                // All working threads
        cxstructs::SlotAllocator<50> jobAllocator;       // Allocator for jobs
        SpinLock queueLock;                              // The lock to make queue access thread safe
        SpinLock workedLock;                             // The lock to worked vector thread safe
        std::atomic<bool> shutDown = false;              // Signal to shut down all threads
        std::atomic<bool> isHibernate = false;           // If the scheduler is running
        std::atomic<int> currentJobsSize = 0;            // Current jobs
        uint16_t handleID = 0;                           // The internal handle counter
        double targetTime = 0;
        double sleepTime = 0;

        ~JobData() { close(); } // Added for safety

        void addWorkedJob(const IJob* job)
        {
            SpinLockGuard guard{workedLock};
            ++currentJobsSize;
            workedJobs.push_back(job);
        }

        void removeWorkedJob(const IJob* job)
        {
            SpinLockGuard guard{workedLock};
            --currentJobsSize;
            std::erase(workedJobs, job);
            jobAllocator.free(job);
        }

        void onEachTick()
        {
#if MAGIQUE_WORKER_THREADS == 0
            for (auto it = queuedJobs.begin(); it != queuedJobs.end();)
            {
                if ((*it)->execTime <= EngineGetTime())
                {
                    (*it)->run();
                    it = queuedJobs.erase(it);
                }
                else
                    ++it;
            }
#endif
        }

        void close()
        {
            shutDown = true;
            isHibernate = true;
            for (auto& t : threads)
            {
                if (t.joinable())
                {
                    t.join();
                }
            }
            threads.clear();
            jobAllocator.destroy();
        }

        static void WorkerThreadFunc(JobData* scheduler)
        {
            bool waited = false;
            while (!scheduler->shutDown.load(std::memory_order::acquire))
            {
                while (!scheduler->isHibernate.load(std::memory_order::acquire))
                {
                    waited = false;
                    SpinLockGuard guard{scheduler->queueLock};
                    if (!scheduler->queuedJobs.empty())
                    {
                        auto* job = scheduler->queuedJobs.front();
                        if (job->execTime > EngineGetTime()) [[unlikely]]
                            continue;
                        scheduler->queuedJobs.pop_front();
                        MAGIQUE_ASSERT(job->id != JobID::null, "Null handle");
                        job->run();
                        scheduler->removeWorkedJob(job);
                    }
                    std::this_thread::yield();
                }

                // Wait more precisely with spinning at the end - only if necessary
                if (!waited)
                {
                    if (GetTime() + scheduler->sleepTime < scheduler->targetTime)
                        WaitTime(scheduler->targetTime, scheduler->sleepTime);
                    waited = true;
                }
                std::this_thread::yield();
            }
        }
    };


    namespace global
    {
        inline JobData SCHEDULER{};
    }

} // namespace magique

#endif // MAGIQUE_JOB_SCHEDULER_H
