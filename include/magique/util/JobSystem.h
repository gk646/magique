// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_JOBSYSTEM_H
#define MAGIQUE_JOBSYSTEM_H

#include <magique/fwd.hpp>
#include <tuple>

//===============================================
// Job System
//===============================================
// .....................................................................
// Note: This is for advanced module.
// This system is trimmed for speed by busy waiting during the tick to quickly pickup tasks.
// Between ticks, it's in hibernation, sleeping until woken up again (if not used).
// Allows to submit concurrent jobs to distribute compatible work across threads and await their completion.
// Per default has MAGIQUE_WORKER_THREADS many worker threads.
// Note: Don't forget to give the main thread work BEFORE waiting for the jobs to return!
// .....................................................................

namespace magique
{
    // Handle to a job
    enum class jobHandle : uint16_t
    {
        null = UINT16_MAX, // The null handle
    };

    //================= JOBS =================//

    // Creates a new job from a lambda or function
    // Note: Only use the returned job pointer to submit jobs
    template <typename Callable>
    IJob* CreateJob(Callable callable);

    // Creates a new job with explicitly given arguments
    // Note: Only use the returned job pointer to submit jobs
    template <typename Callable, typename... Args>
    IJob* CreateExplicitJob(Callable callable, Args... args);

    //================= ADDING =================//
    // Note: Takes ownership of all passed pointers (should not be accessed after)

    // Adds a new job to the global queue
    jobHandle AddJob(IJob* job);

    //================= WAITING =================//

    // Waits till the specified job is completed if it exists
    void AwaitJob(jobHandle handle);

    // Awaits the completion of all given handles if they exist
    // Allows for std::vector<>, std::array<>, and std::initializer_list<>
    template <typename Iterable>
    void AwaitJobs(const Iterable& handles);

    // Awaits the completion of all current tasks
    void AwaitAllJobs();

    //================= LIFECYCLE =================//
    // Note: Called automatically when using the game template - ONLY call if your not using the game template!

    // Brings all workers back to speed (out of hibernate)
    void WakeUpJobs();

    // Puts all workers to hibernation - pass the target until which to hibernate and the actual sleep time
    void HibernateJobs(double target, double sleepTime);

    //================= JOBS =================//

    // Job base class - allows to call templated lambdas
    struct IJob
    {
        virtual ~IJob() = default;
        virtual void run() = 0;
        jobHandle handle = jobHandle::null;
    };

    // Allows to explicitly specify parameters
    template <typename Func, typename... Args>
    struct ExplicitJob final : IJob
    {
        explicit ExplicitJob(Func func, Args... args);
        void run() override;

    private:
        Func func;
        std::tuple<Args...> args;
    };

    template <typename Callable>
    struct Job final : IJob
    {
        explicit Job(Callable func);
        void run() override;

    private:
        Callable func_;
    };

} // namespace magique


//================= IMPLEMENTATION =================//


namespace magique
{
    namespace internal
    {
        // Initializes the job system
        // Note: Does not have to be called manually when using the game template
        bool InitJobSystem();

        // Closes the job system
        // Note: Does not have to be called manually when using the game template
        bool CloseJobSystem();

        void* GetJobMemory(size_t bytes);
    } // namespace internal

    template <typename Callable>
    IJob* CreateJob(Callable callable)
    {
        constexpr auto size = sizeof(Job<Callable>);
        void* ptr = internal::GetJobMemory(size);
        return new (ptr) Job<Callable>(callable);
    }

    template <typename Callable, typename... Args>
    IJob* CreateExplicitJob(Callable callable, Args... args)
    {
        constexpr auto size = sizeof(ExplicitJob<Callable, Args...>);
        void* ptr = internal::GetJobMemory(size);
        return new (ptr) ExplicitJob<Callable, Args...>(callable, args...);
    }

    template <typename Func, typename... Args>
    ExplicitJob<Func, Args...>::ExplicitJob(Func func, Args... args) :
        func(std::move(func)), args(std::make_tuple(args...))
    {
    }

    template <typename Func, typename... Args>
    void ExplicitJob<Func, Args...>::run()
    {
        std::apply(func, args);
    }

    template <typename Callable>
    Job<Callable>::Job(Callable func) : func_(std::move(func))
    {
    }

    template <typename Callable>
    void Job<Callable>::run()
    {
        func_();
    }
} // namespace magique
#endif //MAGIQUE_JOBSYSTEM_H