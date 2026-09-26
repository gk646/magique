// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ECSOBSERVER_H
#define MAGIQUE_ECSOBSERVER_H

#include <magique/magique.hpp>

enum class EntityType : uint16_t
{
    Player,
};

struct Example final : Game
{
    std::string reverse;

    void onStartup(AssetLoader& loader) override
    {
        EntityCreate(EntityType::Player, {}, {}, 15, false);
        EntityRegister(EntityType::Player, [](Entity e, EntityType type) { ComponentGive<CollisionC>(e); });
    }

    void onUpdateGame(GameState gameState) override
    {
        auto player = EntityFindFirstOf(EntityType::Player).value_or(NullEntity);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            ComponentGet<PositionC>(player).pos += 5;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            EntityPatchDiff<PositionC>(player, {reverse});
            LOG_INFO("Revert to old state: %s", ToJSON(ComponentGet<PositionC>(player).pos).data());
        }

        if (IsKeyPressed(KEY_R))
            ObserverStateClear();

        EntityOnChange<PositionC>(player,
                                  [&](entt::entity e, const PositionC& oldPos, const PositionC& newPos)
                                  {
                                      std::string old = std::string{ToJSON(oldPos.pos)};
                                      LOG_INFO("Changed from %s to %s", old.c_str(), ToJSON(newPos.pos).data());
                                      reverse = ObserverDiff(newPos, oldPos);
                                      LOG_INFO("Size: %d bytes", reverse.size());
                                  });

        auto& position = ComponentGet<PositionC>(player);
        if (IsKeyPressed(KEY_W))
            position.pos.y -= 100.0F;
        if (IsKeyPressed(KEY_A))
            position.pos.x -= 100.0F;
        if (IsKeyPressed(KEY_S))
            position.pos.y += 100.0F;
        if (IsKeyPressed(KEY_D))
            position.pos.x += 100.0F;
    }

    void onDrawGame(GameState gameState, Camera2D& camera2D) override
    {
        auto player = EntityFindFirstOf(EntityType::Player).value();
        const auto& position = ComponentGet<PositionC>(player);
        DrawRectangleRec(Rect{position, {50}}, RED);
    }
};


#endif // MAGIQUE_ECSOBSERVER_H
