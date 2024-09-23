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
    struct Teleporter final
    {
        float x, y, w, h;
        float outX, outY;
        MapID origin;
        MapID destination;
    };
    inline static std::vector<Teleporter> teleporters; // Should be a hashmap but vector is easier

    static void setup();
    static void update();
};

#endif //SYSTEMS_H