#include <array>

#include <magique/util/Jobs.h>
#include <magique/internal/Macros.h>
#include <magique/util/Logging.h>
#include <magique/util/Defines.h>

#include "internal/headers/IncludeWindows.h"
#include "internal/globals/JobScheduler.h"

static bool initCalled = false;

namespace magique
{
    bool InitJobSystem()
    {
        if (initCalled)
        {
            LOG_WARNING("Init called twice. Skipping...");
            return true;
        }
        initCalled = true;
        auto& scd = global::SCHEDULER;
        scd.shutDown = false;
        scd.isHibernate = true;
        for (int i = 0; i < MAGIQUE_WORKER_THREADS; ++i) // 2 Worker Threads + 1 Main Thread + 1 Update Thread = 4
        {
            scd.threads.emplace_back(WorkerThreadFunc, &global::SCHEDULER, 2 + i);
        }
        return true;
    }

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

    jobHandle AddGroupJob(IJob* job, int group) { return jobHandle::null; }

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

    using WorkArray = std::array<jobHandle,MAGIQUE_WORKER_THREADS +1>;
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
        ++scd.usingThreads;
        scd.isHibernate = false;
    }

    void HibernateJobs(const double startTime, const double tickTime)
    {
        auto& scd = global::SCHEDULER;
        scd.startTime = startTime;
        scd.tickTime = tickTime;
        --scd.usingThreads;
        M_ASSERT(scd.usingThreads >= 0, "Mismatch between wakup() and hibernate() calls!");
        if (scd.usingThreads == 0)
        {
            scd.isHibernate = true;
        }
    }


} // namespace magique