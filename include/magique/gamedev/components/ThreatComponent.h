#ifndef MAGIQUE_THREATCOMPONENT_H
#define MAGIQUE_THREATCOMPONENT_H

#include <magique/internal/InternalTypes.h>

//===============================
// Threat Component
//===============================
// ................................................................................
// This component is useful to simulate dynamic targeting behavior for enemies.
// Threat is a concept of games like World of Warcraft where other entities (e.g. players or even other monsters)
// generate threat based on their actions. The enemy then targets the highest threat target.
// ................................................................................

namespace magique
{
    // Feel free to change that
    // Default: equal to the maximum amount of possible players
    inline constexpr int THREAT_TABLE_SIZE = MAGIQUE_MAX_PLAYERS;

    struct ThreatC final
    {
        ThreatC(int aggroRange, int chaseRange);

        // Adds the given amount of threat to the target - both positive and negative threat
        // If the target does not exist it's added to the table
        // If the targets threat goes below 0 it is removed from the table
        void addThreat(entt::entity target, float threat);

        // Returns true if the threat table is full
        [[nodiscard]] bool getIsTableFull() const;

    private:
        internal::ThreatTableEntry table[THREAT_TABLE_SIZE];
        uint16_t aggroRange;
        uint16_t chaseRange;
        uint8_t targets;
    };
} // namespace magique
#endif //MAGIQUE_THREATCOMPONENT_H