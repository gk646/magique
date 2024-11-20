#ifndef MAGIQUE_AI_AGENT_H
#define MAGIQUE_AI_AGENT_H

#include <vector>
#include <cmath>
#include <cfloat>
#include <magique/internal/InternalTypes.h>
#include <magique/util/RayUtils.h>
#include <magique/util/Math.h>

//===============================
// AI Agent Module
//===============================
// ................................................................................
// Note: This is an advanced module!
// This module allows to define, pretrain and deploy a learning agent (modified Q-Learning).
// The core concept is that of a state. The agent starts in the given state and picks the best one depending on incoming events.
// It uses 'online-learning' (adapts to incoming events) by adjusting the scores of states based on the provided score.
// The user has to provide a rating of the decision (the chosen state) only when handling the next event.
// This allows to defer the score calculations and observe the outcome of the state (Boss fight agent):
//      Event PLAYER_CLOSE -> Agent picks state DEFEND -> boss blocks attacks (high score) -> score is added on next event
//
// The usage of enums allows to abstract away numeric information like: PlayerDistance: 100 -> PLAYER_MEDIUM_DISTANCE
// This still allows to define complex behavior but with an interface that's easier to understand and use.
//
// Note: All scores have to be provided in the chosen range (in the constructor). User util/Math.h for scaling if needed
// ................................................................................

namespace magique
{
    // IMPORTANT: Passed enums need to have the ENUM_SIZE as their last member
    template <class StateEnum, class EventEnum>
    struct AIAgent final
    {
        using Memory = internal::AgentMemory<StateEnum, EventEnum>;

        //================= SETUP =================//

        // Creates a new agent in the given initial state
        explicit AIAgent(StateEnum initialState, float valueRange);

        // Initializes all state scores with random values between min and max (included)
        void initialize(float min, float max);

        // Sets the score for the given state when the given event occurs - must be inside the chosen 'valueRange'
        // This is later used to pick the best state for the given event - the higher, the better (maximum is picked)
        // Note: Should be used at the start to set up the basic strategy - can be used later on to reset scores
        //      - stateScore: the score of the state in reaction to the given event
        void setScore(StateEnum state, EventEnum event, float stateScore);

        //================= USAGE =================//

        // Returns the best state in reaction to the given event and learns from the provided stateScore
        //      - stateScore: the score (success) of current state since the last time this method was called
        StateEnum handleNewEvent(EventEnum event, float stateScore);

        // Returns the current state of the agent
        StateEnum getCurrentState() const;

        // Returns the last event that was handled - EventEnum::ENUM_SIZE when no event was handled yet
        EventEnum getLastEvent() const;

        // Sets how strong the learn effect is from a given reward (both positive or negative)
        // Default: 0.5
        void setLearnRate(float learnRate);

        //================= RULES =================//
        // Note: Rules are useful to define fixed behavior regardless of the learning progress (e.g. boss patterns, ...)
        //       They are evaluated BEFORE the agents decision and skip it in case a matching rule is found


    private:
        void learnMemory(const Memory& memory);
        StateEnum findBestState(EventEnum event);
        void saveMemory(const Memory& memory) { memories.add(memory); }
        static_assert(std::is_enum_v<StateEnum>, "StateEnum must be an enum");
        static_assert(std::is_enum_v<EventEnum>, "EventEnum must be an enum");

        internal::CircularBuffer<Memory> memories{50};
        internal::StaticMatrix<float, (int)EventEnum::ENUM_SIZE, (int)StateEnum::ENUM_SIZE> qMatrix;
        Memory currentMemory;
        const float valueRange;       // valid score and q-score range
        float learnRate = 0.6F;       // or alpha
        float progressiveness = 0.1F; // or gamma - how much we value potential future rewards
        StateEnum currentState{};
    };

} // namespace magique

//================= IMPLEMENTATION =================//

namespace magique
{
    template <class StateEnum, class EventEnum>
    AIAgent<StateEnum, EventEnum>::AIAgent(StateEnum initialState, const float valueRange) :
        valueRange(valueRange), currentState(initialState)
    {
        currentMemory.state = initialState;
        currentMemory.causingEvent = EventEnum::ENUM_SIZE; // Mark as invalid
    }
    template <class StateEnum, class EventEnum>
    void AIAgent<StateEnum, EventEnum>::initialize(const float min, const float max)
    {
        MAGIQUE_ASSERT(min <= max, "Minimum value must be smaller than maximum value ");
        MAGIQUE_ASSERT(min >= -valueRange && max <= valueRange, "Initial values have to be within the valueRange");

        for (int i = 0; i < (int)EventEnum::ENUM_SIZE; ++i)
        {
            for (int j = 0; j < (int)StateEnum::ENUM_SIZE; ++j)
            {
                qMatrix(i, j) = GetRandomFloat(min, max);
            }
        }
    }
    template <class StateEnum, class EventEnum>
    void AIAgent<StateEnum, EventEnum>::setScore(StateEnum state, EventEnum event, float stateScore)
    {
        MAGIQUE_ASSERT(stateScore >= -valueRange && stateScore <= valueRange, "stateScore must be within valueRange");
        qMatrix(static_cast<int>(event), static_cast<int>(state)) = stateScore;
    }
    template <class StateEnum, class EventEnum>
    StateEnum AIAgent<StateEnum, EventEnum>::handleNewEvent(EventEnum event, float stateScore)
    {
        currentMemory.reward = stateScore;
        currentMemory.nextEvent = event;
        learnMemory(currentMemory);
        saveMemory(currentMemory);
        const auto bestState = findBestState(event);
        currentState = bestState;
        currentMemory = {};
        currentMemory.causingEvent = event;
        currentMemory.state = bestState;
        return bestState;
    }
    template <class StateEnum, class EventEnum>
    StateEnum AIAgent<StateEnum, EventEnum>::getCurrentState() const
    {
        return currentState;
    }
    template <class StateEnum, class EventEnum>
    EventEnum AIAgent<StateEnum, EventEnum>::getLastEvent() const
    {
        return currentMemory.causingEvent;
    }
    template <class StateEnum, class EventEnum>
    void AIAgent<StateEnum, EventEnum>::setLearnRate(const float newLearnRate)
    {
        learnRate = newLearnRate;
    }
    template <class StateEnum, class EventEnum>
    void AIAgent<StateEnum, EventEnum>::learnMemory(const Memory& newMemory)
    {
        Memory memory = newMemory;
        if (memory.causingEvent == EventEnum::ENUM_SIZE) // First memory
            return;

        if (memory.reward < 0)
            memory.reward *= 1.3F; // Make wrong decisions stronger

        // Get the best possible score for this state across all possible events
        const float maxFutureQ = qMatrix.getColumnMax(memory.getState());
        // Current matrix value
        const float currentQ = qMatrix(memory.getCausingEvent(), memory.getState());
        // Adjusts based on reward - does not scale into infinity
        float diff = learnRate * (memory.reward + progressiveness * maxFutureQ - currentQ);

        // Scale the diff based on how much it was from the total value range - strengthen higher learning impacts
        float scale = std::pow(1.0F + LerpInverse(0.0F, valueRange, std::abs(diff)), 2);
        diff *= scale;

        // Keep the total qscore within value range
        const float newTempQ = currentQ + diff;
        if (diff >= 0 && newTempQ >= 0)
            diff *= 1.0F - LerpInverse(0.0F, valueRange, newTempQ);
        else if (diff < 0 && newTempQ < 0)
            diff *= LerpInverse(-valueRange, 0.0F, newTempQ);

        // Apply change
        qMatrix(memory.getCausingEvent(), memory.getState()) = currentQ + diff;


        // Theory: last states contribute to the current one -> update them with a fraction of the current reward
        // Problem: A good decision after a wrong one updates the wrong one with good reward -> can lead to agent getting stuck
        constexpr float decayFactor = 0.5F;
        float discount = 0.15F;

        const auto func = [&](const Memory& oldMemory)
        {
            if (oldMemory.causingEvent == EventEnum::ENUM_SIZE) // First event
                return;

            if (oldMemory.reward < 0) // Dont undo bad decisions
                return;

            // Cumulative reward based on both reward and future state's quality
            float cumulativeReward = discount * memory.reward * scale;
            discount *= decayFactor; // Decay future rewards
            qMatrix(oldMemory.getCausingEvent(), oldMemory.getState()) += cumulativeReward;
        };
        memories.forLastN(5, func);
    }
    template <class StateEnum, class EventEnum>
    StateEnum AIAgent<StateEnum, EventEnum>::findBestState(EventEnum event)
    {
        // This just averages out the rewards of all reactions to the given state and weighs it with the current best state
        // Then decides the best reaction to the event - this makes adaption a bit slower but improves long term memory

        constexpr float currentQValueImportance = 0.25; // How much best scores q value contributes to the average
        constexpr int cacheSize = 10;

        StateEnum statesCache[cacheSize];
        std::fill(statesCache, statesCache + cacheSize, StateEnum::ENUM_SIZE);
        float statesCounterCache[cacheSize] = {};
        float statesScoreCache[cacheSize] = {};

        // Average the rewards for different states when it was chosen in reaction to the current event
        const auto func = [&](const Memory& oldMemory)
        {
            if (oldMemory.causingEvent == event)
            {
                for (int i = 0; i < cacheSize; ++i)
                {
                    if (statesCache[i] == StateEnum::ENUM_SIZE || statesCache[i] == oldMemory.state)
                    {
                        statesCache[i] = oldMemory.state;
                        statesScoreCache[i] += oldMemory.reward; // sum the reward for the reaction to the event
                        ++statesCounterCache[i];                 // save how many values contributed to sum
                        return;
                    }
                }
            }
        };
        memories.forLastN(memories.capacity, func);

        StateEnum bestHistoricState = StateEnum::ENUM_SIZE;
        const auto bestCurrentState = StateEnum(qMatrix.getRowMaxIndex(static_cast<int>(event)));
        const auto bestQScore = qMatrix.getRowMax(static_cast<int>(event));

        float bestHistoryScore = FLT_MIN;
        for (int i = 0; i < cacheSize; ++i)
        {
            if (statesCache[i] == StateEnum::ENUM_SIZE)
                continue;

            statesScoreCache[i] /= statesCounterCache[i];

            if (statesCache[i] == bestCurrentState)
                statesScoreCache[i] += bestQScore * currentQValueImportance;

            if (statesScoreCache[i] > bestHistoryScore)
            {
                bestHistoryScore = statesScoreCache[i];
                bestHistoricState = statesCache[i];
            }
        }

        // If no historic state was found, fall back to the best current state
        return bestHistoricState == StateEnum::ENUM_SIZE ? bestCurrentState : bestHistoricState;
    }

} // namespace magique

#endif //MAGIQUE_AI_AGENT_H