// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ACTION_INPUT_H
#define MAGIQUE_ACTION_INPUT_H

#include <magique/core/Types.h>
#include <raylib/raylib.h>

//===============================================
// Input Action Module
//===============================================
// .....................................................................
// This module helps to abstract direct checks of player input (e.g. with IsKeyDown())
// This allows input to come from multiple sources (NPC Decision Making Engine, Network,...)
// This is achieved by having a common class that can be checked with the same methods, but produced in different ways
// Also by designing the system to take abstract actions it easily allows for diverse gameplay options:
// - Can add an effect where the player is controlled by the NPC AI Engine
// - Can easily let the player control any monster or npc entity
// - Doesn't matter if the inputs come over the network or are produced locally
//
// Suggested Workflow:
// - You define common actions (e.g. MOVE_LEFT, MOVE_RIGHT, ABILITY_1) for main entities (player, npcs, ...)
// - You have a component for NPC's and players that saves (Action, InputAction) pairs
// - You update the InputAction each tick depending on the type of entity
//      - The player: Use FromKeybind(), FromKey() - uses the direct polling functions
//      - The NPC's: Some AI component calculate NPC actions and creates the action virtually
//      - The multiplayers: Send their actions over the network
// - Then a single system can uniformly update the movement or abilities for all entities in the same way
// .....................................................................

namespace magique
{
    // AbstractKeyAction is a common interface where the receiver doesn't care about the specific key
    // You only pass the state of the action, not which key caused it
    // Can be created from many sources
    // Note: Only updated once in the constructor => create new each tick
    struct ActionInput final
    {
        // From keybind using direct polling
        static ActionInput FromKeybind(const Keybind& bind);

        // From a key using direct polling
        static ActionInput FromKey(KeyboardKey key);
        static ActionInput FromKey(MouseButton button);

        // Creates the action from the given values
        static ActionInput FromVirtual(bool pressed, bool down = false, bool released = false);

        // Returns the state of the action
        [[nodiscard]] bool getIsDown() const;
        [[nodiscard]] bool getIsPressed() const;
        [[nodiscard]] bool getIsReleased() const;

    private:
        bool isPressed = false;
        bool isDown = false;
        bool isReleased = false;
    };

} // namespace magique

#endif //MAGIQUE_ACTION_INPUT_H