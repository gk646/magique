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
    magique::CompileAssetImage("../res");
    WizardQuest game{};
    return game.run();

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
}