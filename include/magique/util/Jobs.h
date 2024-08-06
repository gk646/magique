#ifndef MAGIQUE_JOBS_H
#define MAGIQUE_JOBS_H

#include <tuple>
#include <magique/fwd.hpp>

//-----------------------------------------------
// Job System
//-----------------------------------------------
// .....................................................................
// This is for advanced users. This systems is trimmed for speed by busy waiting during the tick to quickly pickup tasks.
// Between ticks its in hibernation, sleeping until woken up again.
// Allows to submit concurrent jobs to distribute compatible work across threads and await their completion.
// Note: Dont forget to give the main thread work BEFORE waiting for the jobs to return!
// Per default has 3 worker threads.
// .....................................................................

namespace magique
{
    // Handle to a job
    enum class jobHandle : uint16_t
    {
        null = UINT16_MAX, // The null handle
    };

    // Initializes the job system
    // Note: Does not have to be called manually when using the game template
    bool InitJobSystem();

    //----------------- JOBS -----------------//

    // Creates a new job from a lambda or function
    // Note: Only use the returned job pointer to submit jobs
    template <typename Callable>
    IJob* CreateJob(Callable callable);

    // Creates a new job with explicitly given arguments
    // Note: Only use the returned job pointer to submit jobs
    template <typename Callable, typename... Args>
    IJob* CreateExplicitJob(Callable callable, Args... args);

    //----------------- ADDING -----------------//
    // Note: Takes ownership of all passed pointers (should not be accessed after)

    // Adds a new job to the global queue
    jobHandle AddJob(IJob* job);

    // Adds the job to a group
    //jobHandle AddGroupJob(IJob* job, int group);

    //----------------- WAITING -----------------//

    // Waits till the specified job is completed if it exists
    void AwaitJob(jobHandle handle);

    // Awaits the completion of all given handles if they exist
    // Allows for std::vector<>, std::array<>, and std::initializer_list<>
    template <typename Iterable>
    void AwaitJobs(const Iterable& handles);

    // Awaits the completion of all current tasks
    void AwaitAllJobs();

    //----------------- LIFECYCLE -----------------//
    // Note: Called automatically when using the game tempalte - ONLY call if your not using the game template!

    // Brings all workers back to speed (out of hibernate)
    void WakeUpJobs();

    // Puts all workers to hibernation - pass the target until which to hibernate and the actual sleep time
    void HibernateJobs(double target, double sleepTime);

    //----------------- JOBS -----------------//

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


//----------------- IMPLEMENTATION -----------------//

template <typename Callable>
magique::Job<Callable>::Job(Callable func) : func_(std::move(func))
{
}
template <typename Callable>
void magique::Job<Callable>::run()
{
    func_();
}
template <typename Func, typename... Args>
magique::ExplicitJob<Func, Args...>::ExplicitJob(Func func, Args... args) :
    func(std::move(func)), args(std::make_tuple(args...))
{
}
template <typename Func, typename... Args>
void magique::ExplicitJob<Func, Args...>::run()
{
    std::apply(func, args);
}
template <typename Callable>
magique::IJob* magique::CreateJob(Callable callable)
{
    return new Job(callable);
}
template <typename Callable, typename... Args>
magique::IJob* magique::CreateExplicitJob(Callable callable, Args... args)
{
    return new ExplicitJob(callable, args...);
}

#endif //MAGIQUE_JOBS_H