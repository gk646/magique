#ifndef SCRIPTS_H
#define SCRIPTS_H

#include <magique/ecs/Scripting.h>

using namespace magique;

struct PlayerScript final : EntityScript
{
    void onTick(entt::entity self, bool updated) override;

    void onKeyEvent(entt::entity self) override;
};

struct NetPlayerScript final : EntityScript
{
};

struct TrollScript final : EntityScript
{
    void onTick(entt::entity self, bool updated) override;
};


#endif //SCRIPTS_H