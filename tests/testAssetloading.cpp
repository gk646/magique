// SPDX-License-Identifier: zlib-acknowledgement

#include <catch_amalgamated.hpp>
#include <thread>

#include <magique/assets/AssetLoader.h>
#include <magique/assets/AssetPacker.h>
#include <magique/util/JobSystem.h>

using namespace magique;

struct Setup final
{
    Setup() { CompileAssetImage("./", "test-data.bin"); }

    ~Setup()
    {
        remove("test-data.bin");
        remove("index.magique");
    }
};

TEST_CASE("execution order")
{
    Setup SETUP{};

    internal::InitJobSystem();
    WakeUpJobs();
    AssetLoader loader("test-data.bin", 0);

    std::vector<std::string> executionOrder;
    std::unordered_map<std::string, std::thread::id> threadMap;
    std::mutex orderMutex;

    std::thread::id mainThreadId = std::this_thread::get_id();

    auto registerTestTask = [&](const std::string& name, ThreadType thread, PriorityLevel priority)
    {
        loader.registerTask(
            [&, name, thread](AssetContainer& assets)
            {
                std::lock_guard const lock(orderMutex);
                executionOrder.push_back(name);
                threadMap[name] = std::this_thread::get_id();

                //printf("Task executed: %s\n", name.c_str());

                if (thread == THREAD_MAIN)
                {
                    assert(std::this_thread::get_id() == mainThreadId && "Main thread task not running on main thread!");
                }
            },
            thread, priority);
    };

    registerTestTask("Task A", THREAD_MAIN, CRITICAL);
    registerTestTask("Task B", THREAD_ANY, HIGH);
    registerTestTask("Task C", THREAD_MAIN, MEDIUM);
    registerTestTask("Task D", THREAD_ANY, MEDIUM);
    registerTestTask("Task E", THREAD_ANY, LOW);
    registerTestTask("Task F", THREAD_MAIN, LOW);

    while (!loader.step())
    {
        // Simulate task execution
    }

    REQUIRE(executionOrder.size() == 6);

    REQUIRE(executionOrder[0] == "Task A");
    REQUIRE(executionOrder[1] == "Task B");
    REQUIRE((executionOrder[2] == "Task C" || executionOrder[2] == "Task D"));
    REQUIRE((executionOrder[3] == "Task C" || executionOrder[3] == "Task D"));
    REQUIRE((executionOrder[4] == "Task E" || executionOrder[4] == "Task F"));
    REQUIRE((executionOrder[5] == "Task E" || executionOrder[5] == "Task F"));

    for (const auto& taskName : {"Task A", "Task C", "Task F"})
    {
        REQUIRE(threadMap[taskName] == mainThreadId);
    }
}