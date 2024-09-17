#include "WizardQuest.h"

#include <chrono>
#include <magique/assets/AssetPacker.h>
#include <magique/gamedev/AIAgent.h>
#include <magique/gamedev/ShareCode.h>

int main()
{
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

    AIAgent<TradingState, TradingEvents> controller{TradingState::HOLD,100};

    controller.initialize(-1, 1);

    // Pretrain the agent to HOLD during panic and SELL when either group sells
    controller.setScore(TradingState::HOLD, TradingEvents::PANIC_EVENT, 10);   // Hold during panic
    controller.setScore(TradingState::SELL, TradingEvents::GROUP_1_SELLS, 10); // Sell if group 1 sells
    controller.setScore(TradingState::BUY, TradingEvents::GROUP_1_BUYS, 10);   // Sell if group 1 sells
    controller.setScore(TradingState::SELL, TradingEvents::GROUP_2_SELLS, 10); // Sell if group 2 sells

    // Test simulation with various events
    printf("Initial State: %d\n", (int)controller.getCurrentState());

    // Handle GROUP_1_BUYS event
    auto state = controller.handleNewEvent(TradingEvents::GROUP_1_BUYS, 5);
    printf("After GROUP_1_BUYS Event: %d\n", (int)controller.getCurrentState());

    float score = state == TradingState::BUY ? 10 : 0;

    // Handle GROUP_1_SELLS event
    state = controller.handleNewEvent(TradingEvents::GROUP_1_SELLS, score);
    printf("After GROUP_1_SELLS Event: %d\n", (int)state);

    // Handle GROUP_2_BUYS event
    state = controller.handleNewEvent(TradingEvents::GROUP_2_BUYS, 10);
    printf("After GROUP_2_BUYS Event: %d\n", (int)state);

    // Handle GROUP_2_SELLS event
    state = controller.handleNewEvent(TradingEvents::GROUP_2_SELLS, 10);
    printf("After GROUP_2_SELLS Event: %d\n", (int)state);

    // Handle PANIC_EVENT
    state = controller.handleNewEvent(TradingEvents::PANIC_EVENT, 10);
    printf("After PANIC_EVENT: %d\n", (int)state);

    return 0;
    magique::CompileImage("../res");
    WizardQuest game{};
    return game.run();
}