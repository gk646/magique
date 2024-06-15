#include <magique/util/Jobs.h>
#include <magique/util/Macros.h>

#include <cxutil/cxtime.h>

namespace magique
{
    std::mutex mutex;
    void workerThread(Scheduler* scheduler)
    {
        while (!scheduler->shutDown.load(std::memory_order_acquire))
        {
            std::unique_lock lock(mutex, std::defer_lock);
            scheduler->condition.wait(lock,[=]
            {
                return !scheduler->isHibernate;
            });


            while (!scheduler->isHibernate.load(std::memory_order_acquire))
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
            }
        }
    }


    Scheduler::Scheduler(const int threadCount) : mainID(std::this_thread::get_id())
    {
        shutDown = false;
        isHibernate = true;
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

    void Scheduler::wakeup()
    {
        isHibernate.store(false, std::memory_order_release);
        condition.notify_all();
    }

    void Scheduler::hibernate()
    {
        isHibernate.store(true, std::memory_order_release);
    }
} // namespace magique