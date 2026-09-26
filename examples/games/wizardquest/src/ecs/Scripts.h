#ifndef SCRIPTS_H
#define SCRIPTS_H

#include <magique/ecs/Scripting.h>

struct PlayerScript final : EntityScript
{
    void onUpdate(entt::entity self, bool updated) override;
};

struct NetPlayerScript final : EntityScript
{
};

struct TrollScript final : EntityScript
{
    void onUpdate(entt::entity self, bool updated) override;
};


#endif // SCRIPTS_H
