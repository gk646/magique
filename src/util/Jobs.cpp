#include <magique/util/Jobs.h>
#include <magique/util/Macros.h>

#include <cxutil/cxtime.h>

namespace magique
{
    // this is allows to wait without using sleep()
    // sleep causes big delays sometimes up to 2ms
    std::mutex mutex;
    void workerThread(Scheduler* scheduler)
    {
        while (!scheduler->shutDown.load(std::memory_order_acquire))
        {
            {
                std::unique_lock lock(mutex);
                scheduler->condition.wait(lock, [=] { return !scheduler->isHibernate; });
            }

            while (!scheduler->isHibernate.load(std::memory_order_acquire))
            {
                scheduler->queueLock.lock();
                if (!scheduler->jobQueue.empty())
                {
                    //printf("Taken by: %d\n", (int)std::this_thread::get_id());
                    const auto job = scheduler->jobQueue.front();
                    scheduler->jobQueue.pop_front();
                    scheduler->queueLock.unlock();
                    M_ASSERT(job->handle != jobHandle::null, "Null handle");
                    // cxstructs::now(1);
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
            std::this_thread::sleep_for(std::chrono::microseconds(10));
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
        isHibernate = true;
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


    void Scheduler::awaitAll() const
    {

        while (currentJobsSize > 0)
        {
        }
    }


    void Scheduler::wakeup()
    {
        ++usingThreads;
        isHibernate = false;
        condition.notify_all();
    }

    void Scheduler::hibernate()
    {
        --usingThreads;
        M_ASSERT(usingThreads >= 0, "Mismatch between wakup() and hibernate() calls!");
        if (usingThreads == 0)
        {
            isHibernate = false;
        }
    }


} // namespace magique