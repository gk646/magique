// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_JOBSYSTEM_H
#define MAGIQUE_JOBSYSTEM_H

#include <functional>
#include <span>
#include <magique/fwd.hpp>

//===============================================
// Job System
//===============================================
// .....................................................................
// This system is trimmed for speed by busy waiting during the tick to quickly pickup tasks.
// Between ticks, it's in hibernation, sleeping until woken up again (if not used).
// Allows to submit concurrent jobs to distribute compatible work across threads and await their completion.
// Per default has MAGIQUE_WORKER_THREADS many worker threads.
// Note: Don't forget to give the main thread work as well BEFORE waiting for the jobs to return!
// .....................................................................

namespace magique
{
    // Handle to a job
    enum class JobID : uint16_t
    {
        null = UINT16_MAX, // The null handle
    };

    // Adds a new job from the given callable or function
    // Called at least after the given delay passed (seconds)
    // Note: A callable can be any lambda or std::function() e.g. [captures](arguments){ return myVar + 3;}, ...
    template <typename Callable>
    JobID JobAdd(Callable callable, float delay = 0.0F);

    // Allows to specify explicit arguments
    template <typename Callable, typename... Args>
    JobID JobAddEx(Callable callable, Args... args);

    // Waits till the specified jobs are completed
    void JobAwait(JobID id);
    void JobAwait(std::span<const JobID> handles);

    // Waits until all jobs are done (e.g. none exists) - might never be true if new ones are added
    void JobAwaitAll();

} // namespace magique


//================= IMPLEMENTATION =================//


namespace magique
{
    namespace internal
    {
        // Initializes the job system
        // Note: Does not have to be called manually when using the game template
        bool JobsInit();

        // Closes the job system
        // Note: Does not have to be called manually when using the game template
        bool JobsClose();

        // Brings all workers back to speed (out of hibernate)
        void JobsWakeUp();

        // Puts all workers to hibernation - pass the target until which to hibernate and the actual sleep time
        void JobsSleep(double target, double sleepTime);

        JobID JobQueue(IJob* job, float delay = 0.0F);
        void* JobGetJobMemory(size_t bytes);

    } // namespace internal

    // Job base class - allows to call templated lambdas
    struct IJob
    {
        virtual ~IJob() = default;
        virtual void run() = 0;
        JobID id = JobID::null;
        float execTime = 0.0F;
    };

    // Allows to explicitly specify parameters
    template <typename Func, typename... Args>
    struct ExplicitJob final : IJob
    {
        explicit ExplicitJob(Func func, Args... args) : func(std::move(func)), args(std::make_tuple(args...)) {}
        void run() override { std::apply(func, args); }

    private:
        Func func;
        std::tuple<Args...> args;
    };

    template <typename Callable>
    struct Job final : IJob
    {
        explicit Job(Callable func) : func_(std::move(func)) {}
        void run() override { func_(); }

    private:
        Callable func_;
    };

    template <typename Callable>
    JobID JobAdd(Callable callable, float delay)
    {
        constexpr auto size = sizeof(Job<Callable>);
        void* ptr = internal::JobGetJobMemory(size);
        auto job = new (ptr) Job<Callable>(callable);
        return internal::JobQueue(job, delay);
    }

    template <typename Callable, typename... Args>
    JobID JobAddEx(Callable callable, Args... args)
    {
        constexpr auto size = sizeof(ExplicitJob<Callable, Args...>);
        void* ptr = internal::JobGetJobMemory(size);
        auto job = new (ptr) ExplicitJob<Callable, Args...>(callable, args...);
        return internal::JobQueue(job);
    }

} // namespace magique

#endif // MAGIQUE_JOBSYSTEM_H
