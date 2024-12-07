#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <magique/core/Types.h>

using namespace magique;

struct EntityStatsC final
{
    static constexpr float MAX_HEALTH = 10.0F;
    static constexpr float MAX_MANA = 5.0F;

    float health = MAX_HEALTH;
    float mana = MAX_MANA;
    float maxHealth = MAX_HEALTH;
    float maxMana = MAX_MANA;
    float moveSpeed = 2.5F;

    float getManaPercent() const { return mana / maxMana; }
    float getHealthPercent() const { return health / maxHealth; }
};

struct MovementC final
{
    static constexpr float DECAY_FACTOR = 0.9;
    static constexpr float STOP_THRESHOLD = 0.01F;
    static constexpr int TELEPORT_COOLDOWN = 60; // 1 second

    float baseVelocX, baseVelocY;     // Movement velocity - reset each frame
    float extVelocX, extVelocY;       // External velocity (forces)  - decays slowly
    bool movedLeft = false;           // For animation flipping
    int teleportCooldownCounter = 30; // So you don't get stuck on the teleporter

    Point getVelocity()
    {
        const float totalDx = baseVelocX + extVelocX;
        const float totalDy = baseVelocY + extVelocY;

        // Apply decay only to the external velocity to simulate friction or drag
        extVelocX *= DECAY_FACTOR;
        extVelocY *= DECAY_FACTOR;

        // Reset base velocity after applying to avoid it spilling into next frame
        baseVelocX = 0;
        baseVelocY = 0;

        // Check if the external velocity is low enough to be stopped
        if (std::abs(extVelocX) < STOP_THRESHOLD && std::abs(extVelocY) < STOP_THRESHOLD)
        {
            extVelocX = 0;
            extVelocY = 0;
        }
        ++teleportCooldownCounter;
        return {totalDx, totalDy};
    }
};


#endif //COMPONENTS_H