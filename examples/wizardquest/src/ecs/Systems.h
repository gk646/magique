#ifndef SYSTEMS_H
#define SYSTEMS_H

struct MovementSystem final
{
    static void update();
};


struct AnimationSystem final
{
    static void update();
};

struct TeleportSystem final
{
   inline static std::vector<magique::TileObject> teleporters; // Should be a hashmap but vector is easier

    static void setup();
    static void update();
};

#endif //SYSTEMS_H