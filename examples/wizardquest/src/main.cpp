#include "WizardQuest.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <magique/assets/AssetPacker.h>
#include <magique/gamedev/AIAgent.h>
#include <magique/gamedev/ShareCode.h>
#include <magique/gamedev/VirtualClock.h>

int main()
{
    VirtualClock clock{50}; // A virtual day lasts 50 real minutes

    // Virtual seconds passed for every real second
    float virtualSecondsPerRealSecond = 1440.0F / 50.0F;

    // Calculate the passed real seconds (0.016 seconds per tick)
    float passedRealSeconds = static_cast<float>(0) * (1 / 60.0F);
    printf("Passed Real Seconds: %d\n", static_cast<int>(passedRealSeconds));

    // Calculate how many virtual seconds should have passed given the real seconds
    float expectedVirtualSeconds = passedRealSeconds * virtualSecondsPerRealSecond;
    printf("Expected Virtual Seconds: %d\n", static_cast<int>(expectedVirtualSeconds));

    // Get the actual virtual seconds from the clock
    printf("Passed Virtual Seconds: %d\n", clock.getPassedSeconds());

    // Calculate the difference between expected and actual virtual seconds
    int diff = static_cast<int>(expectedVirtualSeconds - clock.getPassedSeconds());
    printf("Diff: %d\n", diff);

    for (int i = 0; i < 100; ++i)
    {
        clock.syncTimeOfDay();
        // Print the current virtual time
        printf("Day: %d, Hour: %d, Minute: %d, Second: %d\n", clock.getDay(), clock.getHour(), clock.getMinute(),
               clock.getSecond());
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }


    return 0;

    enum class TradingState
    {
        HOLD,
        SELL,
        BUY,
        ENUM_SIZE
    };

    enum class TradingEvents
    {
        GROUP_1_BUYS,
        GROUP_1_SELLS,
        GROUP_2_BUYS,
        GROUP_2_SELLS,
        PANIC_EVENT,
        ENUM_SIZE
    };

    AIAgent<TradingState, TradingEvents> controller{TradingState::HOLD, 100};

    printf("Size: %d\n", sizeof(controller));
    controller.initialize(-1, 1);


    int correctDecisions = 0;
    int incorrectDecisions = 0;
    int stageChangeIteration = 1000;
    int adaptationTime = -1;
    bool adapted = false;

    volatile TradingState state{};
    int stage = 1;
    // Run the simulation for 2000 iterations

    for (int i = 0; i < 2000; ++i)
    {
        if (i == stageChangeIteration)
        {
            // Transition to Stage 2: Change strategy
            stage = 2;
            adapted = false;
        }

        float score;

        if (stage == 1) // After panic
            score = state == TradingState::SELL ? 10 : -10;
        else
            score = state == TradingState::HOLD ? 10 : -10;

        state = controller.handleNewEvent(TradingEvents::GROUP_1_BUYS, score);
        if (score == 10)
            correctDecisions++;
        else
            incorrectDecisions++;

        if (stage == 1) // after g1 buy
            score = state == TradingState::BUY ? 10 : -10;
        else
            score = state == TradingState::SELL ? 10 : -10;

        state = controller.handleNewEvent(TradingEvents::GROUP_1_SELLS, score);
        if (score == 10)
            correctDecisions++;
        else
            incorrectDecisions++;

        if (stage == 1) // after g1 sell
            score = state == TradingState::SELL ? 10 : -10;
        else
            score = state == TradingState::BUY ? 10 : -10;

        state = controller.handleNewEvent(TradingEvents::GROUP_2_BUYS, score);
        if (score == 10)
            correctDecisions++;
        else
            incorrectDecisions++;

        if (stage == 1) // after g2 buy
            score = state == TradingState::BUY ? 10 : -10;
        else
            score = state == TradingState::SELL ? 10 : -10;

        state = controller.handleNewEvent(TradingEvents::GROUP_2_SELLS, score);
        if (score == 10)
            correctDecisions++;
        else
            incorrectDecisions++;

        if (stage == 1) // after g2 sell
            score = state == TradingState::SELL ? 10 : -10;
        else
            score = state == TradingState::BUY ? 10 : -10;

        state = controller.handleNewEvent(TradingEvents::PANIC_EVENT, score);
        if (score == 10)
            correctDecisions++;
        else
            incorrectDecisions++;

        // Record the time to adaptation after stage change
        if (stage == 2 && !adapted && correctDecisions > incorrectDecisions)
        {
            adaptationTime = i - stageChangeIteration;
            adapted = true;
        }
    }

    std::cout << "Correct Decisions: " << correctDecisions << std::endl;
    std::cout << "Incorrect Decisions: " << incorrectDecisions << std::endl;
    std::cout << "Adaptation Time (Iterations after Stage 2): " << adaptationTime << std::endl;


    return -10;
    magique::CompileImage("../res");
    WizardQuest game{};
    return game.run();
}