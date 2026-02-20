#ifndef MAGIQUE_GAMESYSTEMS_H
#define MAGIQUE_GAMESYSTEMS_H

#include <array>
#include <string>
#include <magique/fwd.hpp>

//===============================================
// Gamesystem Module
//===============================================
// ................................................................................
// A gamesystem operates on components and contains all the logic that makes up the game
// This separates concerns, as in the ECS architecture components contain only information
// and systems the logic.
// It is recommended to split draw and update work into separate systems (to keep it simple there is just 1 system class)
// Splitting by system also allows for easy benchmarking:
//      - For each system the average time is tracked for each event method
// Make sure to have enough modular systems so the data is meaningful and its also good practice to do so
// ................................................................................

namespace magique
{
    // Registers the given system - systems are called in the order they are added
    // Note: Methods are only called when the current gamestate is part of the passed "validStates"
    //       empty means all states
    void GameSystemRegister(GameSystem* system,  std::string_view name, std::initializer_list<GameState> stats = {});

    // Prints a table that show average time of each function call for each system
    //      - cutoff: Minimal duration of the system in milliseconds in order to be printed
    // Default: Called on Game::onShutdown()
    void GameSystemPrintStats(float cutoff = 0.01);

    // If true all systems are benchmarked - existing data is cleared on enabling
    // Default: Enabled if in debug mode (MAGIQUE_DEBUG)
    void GameSystemEnableStats(bool value);

    struct GameSystem
    {
        virtual ~GameSystem() = default;

        // Called once when the system is added
        virtual void init() {}

        // Called at render tickrate
        // Default: Called inside Game::onRenderGame()
        virtual void onDrawGame(GameState gameState, Camera2D& camera) {}

        // Called at update tickrate
        // Default: Called inside Game::onUpdateGame()
        virtual void onUpdateGame(GameState gameState) {}

        // Called at update tickrate
        // Default: Called inside Game::onUpdateEnd()
        virtual void onUpdateEnd(GameState gameState) {}

        // Any event function is only called if this returns true
        virtual bool shouldBeCalled() { return true; }

        // Calls the methods of the systems
        // Note: Automatically called in the default implementation of Game when NOT overloaded
        // MUST be called at the right time:
        //  - CallDraw: Inside onRenderGame()
        //  - CallUpdate: Inside onUpdateGame()
        //  - CallUpdateEnd: Inside onUpdateEnd()
        static void CallDrawGame();
        static void CallUpdateGame();
        static void CallUpdateEnd();

        const char* getName() const;

    private:
        M_MAKE_PUB()
        std::string name;
        std::array<bool, UINT8_MAX + 1> validStates;
    };

} // namespace magique

#endif // MAGIQUE_GAMESYSTEMS_H
