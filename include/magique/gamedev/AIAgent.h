#ifndef MAGIQUE_AI_AGENT_H
#define MAGIQUE_AI_AGENT_H

#include <vector>
#include <magique/internal/InternalTypes.h>
#include <magique/util/Math.h>
#include <magique/util/RayUtils.h>

//-------------------------------
// AI Agent Module
//-------------------------------
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

        //----------------- SETUP -----------------//

        // Creates a new agent in the given initial state
        //      - valueRange: maximum possible value for scores in both + and - direction (-range, +range)
        explicit AIAgent(StateEnum initialState, float valueRange);

        // Initializes all state scores with random values between min and max (included)
        void initialize(float min, float max);

        // Sets the score for the given state when the given event occurs
        // This is later used to pick the best state for the given event - the higher, the better (maximum is picked)
        // Note: Should be used at the start to set up basic strategy - can be used later on to reset
        //      - stateScore: the score of the state in reaction to the given event
        void setScore(StateEnum state, EventEnum event, float stateScore);

        //----------------- USAGE -----------------//

        // Returns the best state in reaction to the given event and learns from the provided stateScore
        //      - stateScore: the score (success) of current state since the last time this method was called
        StateEnum handleNewEvent(EventEnum event, float stateScore);

        // Returns the current state of the agent
        StateEnum getCurrentState() const;

        // Returns the last event that was handled - EventEnum::ENUM_SIZE when no event was handled yet
        EventEnum getLastEvent() const;

        //----------------- RULES -----------------//
        // Note: Rules are useful to define fixed behavior regardless of the learn progress (e.g. boss patterns, ...)
        //       They are evaluated BEFORE the agents decision and skip it in case a matching rule is found


    private:
        void updateQValue(const Memory& memory);
        StateEnum findBestState(EventEnum event);
        void saveMemory(const Memory& memory) { memories.push_back(memory); }
        static_assert(std::is_enum_v<StateEnum>, "StateEnum must be an enum");
        static_assert(std::is_enum_v<EventEnum>, "EventEnum must be an enum");

        std::vector<Memory> memories;
        internal::StaticMatrix<float, (int)EventEnum::ENUM_SIZE, (int)StateEnum::ENUM_SIZE> qMatrix;
        Memory currentMemory;
        const float valueRange;       // valid score and q-score range
        float learnRate = 0.2F;       // or alpha
        float progressiveness = 0.1F; // or gamma - how much we value potential future rewards
        StateEnum currentState{};
    };

} // namespace magique

//----------------- IMPLEMENTATION -----------------//

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
        MAGIQUE_ASSERT(stateScore >= -valueRange && stateScore <= valueRange, "stateScore must be within valueRange");
        currentMemory.reward = stateScore;
        currentMemory.nextEvent = event;
        updateQValue(currentMemory);
        saveMemory(currentMemory);
        const auto bestState = findBestState(event);
        currentState = bestState;
        currentMemory = {};
        currentMemory.causingEvent = event;
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
    void AIAgent<StateEnum, EventEnum>::updateQValue(const Memory& memory)
    {
        const float maxFutureQ = qMatrix.getRowMax(memory.getNextEvent());
        const float currentQ = qMatrix(memory.getNextEvent(), memory.getState());
        const float newQ = currentQ + learnRate * (memory.reward + progressiveness * maxFutureQ - currentQ);
        float diff = newQ - currentQ;
        if (newQ >= 0) {
            diff *= 1.0F - LerpInverse(0.0F, valueRange, newQ);
        } else if (newQ < 0) {
            diff *= 1.0F - LerpInverse(-valueRange, 0.0F, newQ);
        }
        qMatrix(memory.getNextEvent(), memory.getState()) = currentQ + diff;
    }
    template <class StateEnum, class EventEnum>
    StateEnum AIAgent<StateEnum, EventEnum>::findBestState(EventEnum event)
    {
        return StateEnum(qMatrix.getRowMaxIndex(static_cast<int>(event)));
    }

} // namespace magique

#endif //MAGIQUE_AI_AGENT_H