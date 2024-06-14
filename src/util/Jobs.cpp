#include <algorithm>
#include <magique/util/Jobs.h>
#include <magique/util/Macros.h>

namespace magique
{
    void workerThread(Scheduler* scheduler)
    {
        while (!scheduler->shutDown.load(std::memory_order_acquire))
        {
            while (scheduler->running.load(std::memory_order_acquire))
            {
                JobBase* job = nullptr;
                {
                    scheduler->queueLock.lock();
                    if (!scheduler->jobQueue.empty())
                    {
                        job = scheduler->jobQueue.front();
                        M_ASSERT(job->handle != jobHandle::null, "Null handle");
                        scheduler->jobQueue.pop_front();
                    }
                    scheduler->queueLock.unlock();
                }

                if (job)
                {
                    job->run();
                    scheduler->removeWorkedJob(job->handle);
                    delete job;
                }
                else
                {
                    std::this_thread::yield();
                }
            }
            std::this_thread::sleep_for(std::chrono::microseconds(1));
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
        running = false;
        for (auto& t : threads)
        {
            if (t.joinable())
                t.join();
        }
    }


    void Scheduler::start() { running.store(true, std::memory_order_release); }


    void Scheduler::await(const std::initializer_list<jobHandle>& handles)
    {
        M_ASSERT(std::this_thread::get_id() == mainID,
                 "Has to be called from the main thread! Use dependencies for chaining jobs");
        while (true)
        {
            bool allCompleted = true;
            for (const auto handle : handles)
            {
                if (std::ranges::any_of(workedJobs, [=](const JobBase* j) { return j->handle == handle; }))
                {
                    allCompleted = false;
                    break;
                }
            }
            if (allCompleted)
                break;
            // std::this_thread::yield(); // Dont wanna yield the main thread
        }
    }

} // namespace magique