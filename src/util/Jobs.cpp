// SPDX-License-Identifier: zlib-acknowledgement
#include <vector>
#include <algorithm>

#include <magique/util/JobSystem.h>
#include <magique/util/Logging.h>
#include <magique/core/Engine.h>

#include "internal/globals/JobData.h"

namespace magique
{

    static bool IsPresent(JobID id)
    {
        auto& scd = global::SCHEDULER;
        {
            SpinLockGuard guard{scd.queueLock};
            if (std::ranges::any_of(scd.queuedJobs, [&](const IJob* job) { return job->id == id; }))
                return true;
        }
        {
            SpinLockGuard guard{scd.workedLock};
            if (std::ranges::any_of(scd.workedJobs, [&](const IJob* job) { return job->id == id; }))
                return true;
        }
        return false;
    }

    void JobAwait(JobID id)
    {
        while (true)
        {
            if (IsPresent(id)) [[likely]]
                std::this_thread::yield();
            else
                break;
        }
    }

    void JobAwait(std::span<const JobID> handles)
    {
        while (std::ranges::any_of(handles, IsPresent))
        {
            std::this_thread::yield();
        }
    }

    void JobAwaitAll()
    {
        auto& scd = global::SCHEDULER;
        while (true)
        {
            SpinLockGuard guard{scd.queueLock};
            SpinLockGuard guard2{scd.workedLock};
            if (scd.queuedJobs.empty() && scd.workedJobs.empty())
                return;
            else
                std::this_thread::yield();
        }
    }

    namespace internal
    {
        bool JobsInit()
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
            for (int i = 0; i < MAGIQUE_WORKER_THREADS; ++i)
            {
                scd.threads.emplace_back(JobData::WorkerThreadFunc, &global::SCHEDULER);
            }
            return true;
        }

        bool JobsClose()
        {
            global::SCHEDULER.close();
            return true;
        }

        void JobsWakeUp()
        {
            auto& scd = global::SCHEDULER;
            scd.isHibernate = false;
        }

        void JobsSleep(const double target, const double sleepTime)
        {
            auto& scd = global::SCHEDULER;
            scd.targetTime = target;
            scd.sleepTime = sleepTime;
            scd.isHibernate = true;
        }

        JobID JobQueue(IJob* job, float delay)
        {
            auto& scd = global::SCHEDULER;
            SpinLockGuard guard{scd.queueLock};
            const auto handle = static_cast<JobID>(scd.handleID++);
            job->id = handle;
            job->execTime = EngineGetTime() + delay;
            scd.queuedJobs.push_back(job);
            return handle;
        }
    } // namespace internal

    void* internal::JobGetJobMemory(const size_t bytes) { return global::SCHEDULER.jobAllocator.allocate(bytes); }

} // namespace magique
