// SPDX-License-Identifier: zlib-acknowledgement
#include <array>
#include <vector>

#include <magique/util/JobSystem.h>
#include <magique/util/Logging.h>

#include "internal/globals/JobScheduler.h"

namespace magique
{
    jobHandle AddJob(IJob* job)
    {
        auto& scd = global::SCHEDULER;
        const auto handle = scd.getNextHandle();
        job->handle = handle;
        scd.queueLock.lock();
        scd.jobQueue.push_back(job);
        scd.queueLock.unlock();
        scd.addWorkedJob(job);
        return handle;
    }

    jobHandle AddGroupJob(IJob* /**/, int /**/) { return jobHandle::null; }

    template <typename Iterable>
    void AwaitJobs(const Iterable& handles)
    {
        auto& scd = global::SCHEDULER;
        while (scd.currentJobsSize > 0)
        {
            bool allCompleted = true;
            scd.workedLock.lock();
            for (const auto handle : handles)
            {
                for (const auto job : scd.workedJobs)
                {
                    if (job->handle == handle)
                    {
                        allCompleted = false;
                        break;
                    }
                }
                if (!allCompleted)
                    break;
            }
            scd.workedLock.unlock();
            if (allCompleted)
                return;
            std::this_thread::yield();
        }
    }

    using WorkArray = std::array<jobHandle, MAGIQUE_WORKER_THREADS + 1>;
    template void AwaitJobs<WorkArray>(const WorkArray& container);
    template void AwaitJobs<std::vector<jobHandle>>(const std::vector<jobHandle>& container);
    template void AwaitJobs<std::initializer_list<jobHandle>>(const std::initializer_list<jobHandle>& container);

    void AwaitAllJobs()
    {
        auto& scd = global::SCHEDULER;
        while (scd.currentJobsSize > 0)
        {
            // YieldProcessor();
        }
    }

    void WakeUpJobs()
    {
        auto& scd = global::SCHEDULER;
        scd.isHibernate = false;
    }

    void HibernateJobs(const double target, const double sleepTime)
    {
        auto& scd = global::SCHEDULER;
        scd.targetTime = target;
        scd.sleepTime = sleepTime;
        scd.isHibernate = true;
    }

    bool internal::InitJobSystem()
    {
        static bool initCalled = false;
        if (initCalled)
        {
            LOG_WARNING("Init called twice. Skipping...");
            return false;
        }
        initCalled = true;
        auto& scd = global::SCHEDULER;
        scd.shutDown = false;
        scd.isHibernate = true;
        for (int i = 0; i < MAGIQUE_WORKER_THREADS; ++i) // 3 Worker Threads + 1 Main Thread = 4
        {
            scd.threads.emplace_back(WorkerThreadFunc, &global::SCHEDULER, 2 + i);
        }
        LOG_INFO("Initialized JobSystem with %d threads", MAGIQUE_WORKER_THREADS);
        return true;
    }

    bool internal::CloseJobSystem()
    {
        global::SCHEDULER.close();
        return true;
    }

    void* internal::GetJobMemory(const size_t bytes) { return global::SCHEDULER.jobAllocator.allocate(bytes); }

} // namespace magique