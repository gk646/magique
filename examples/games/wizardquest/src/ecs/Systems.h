#ifndef SYSTEMS_H
#define SYSTEMS_H

#include <ankerl/unordered_dense.h>

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


struct MultiplayerSystem final
{
    // Maps outgoing connections to a player in our world (for host)
    inline static HashMap<Connection, entt::entity> networkPlayerMap;

    static void init();
    static void update();
    static void postUpdate();

private:
    static void checkBeginSession();
};
#endif //SYSTEMS_H