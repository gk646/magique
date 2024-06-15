#include <magique/util/Jobs.h>
#include <magique/util/Macros.h>

#include <cxutil/cxtime.h>

namespace magique
{
    void workerThread(Scheduler* scheduler)
    {
        while (!scheduler->shutDown.load(std::memory_order_acquire))
        {
            while (scheduler->isHibernate.load(std::memory_order_acquire))
            {
                scheduler->queueLock.lock();
                if (!scheduler->jobQueue.empty())
                {
                    const auto job = scheduler->jobQueue.front();
                    scheduler->jobQueue.pop_front();
                    scheduler->queueLock.unlock();
                    M_ASSERT(job->handle != jobHandle::null, "Null handle");
                    if (job)
                    {
                        job->run();
                        scheduler->removeWorkedJob(job);
                        delete job;
                    }
                }
                else
                {
                    scheduler->queueLock.unlock();
                }
                std::this_thread::sleep_for(std::chrono::nanoseconds(100));
            }
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            std::this_thread::yield();
        }
    }


    Scheduler::Scheduler(const int threadCount) : mainID(std::this_thread::get_id())
    {
        for (size_t i = 0; i < threadCount; ++i)
        {
            threads.emplace_back(workerThread, this);
        }
    }

    Scheduler::~Scheduler()
    {
        shutDown = true;
        isHibernate = false;
        for (auto& t : threads)
        {
            if (t.joinable())
                t.join();
        }
    }

    jobHandle Scheduler::addJob(IJob* job)
    {
        const auto handle = getNextHandle();
        job->handle = handle;
        queueLock.lock();
        jobQueue.push_back(job);
        queueLock.unlock();
        addWorkedJob(job);
        return handle;
    }

    void Scheduler::wakeup() { isHibernate = false; }

    void Scheduler::hibernate() { isHibernate = true; }
} // namespace magique