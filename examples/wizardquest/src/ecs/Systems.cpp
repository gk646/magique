#include <raylib/raylib.h>

#include <magique/core/Core.h>
#include <magique/ecs/ECS.h>
#include <magique/assets/AssetManager.h>
#include <magique/assets/types/TileMap.h>

#include "WizardQuest.h"
#include "ecs/Systems.h"
#include "ecs/Components.h"

using namespace magique;

void MovementSystem::update()
{
    for (const auto e : GetUpdateEntities())
    {
        if (!EntityHasComponents<PositionC, MovementC>(e))
            continue;
        auto& pos = GetComponent<PositionC>(e);
        auto& mov = GetComponent<MovementC>(e);
        const auto [x, y] = mov.getVelocity();
        pos.x += x;
        pos.y += y;
    }
}

void AnimationSystem::update()
{

    for (const auto e : GetUpdateEntities())
    {
        if (EntityHasComponents<AnimationC>(e))
        {
            auto& anim = GetComponent<AnimationC>(e);
            anim.update();
        }
    }
}

void TeleportSystem::setup()
{
    for (int i = 0; i < (int)MapID::MAPS_END; ++i)
    {
        auto& tileMap = GetTileMap(GetMapHandle(MapID(i)));
        if (tileMap.getObjectLayerCount() == 0)
            continue;

        for (const auto& tileObject : tileMap.getObjects(0))
        {
            std::string objectName = std::string(tileObject.getName());
            if (objectName.find("Teleporter") != std::string::npos)
            {
                TileObject teleporter = tileObject;
                teleporter.x *= 3;
                teleporter.y *= 3;
                teleporter.width *= 3;
                teleporter.height *= 3;
                teleporters.push_back(teleporter);
            }
        }
    }
}

void TeleportSystem::update()
{
    for (const auto e : GetUpdateEntities())
    {
        if (!EntityHasComponents<CollisionC>(e))
            continue;
        auto& pos = GetComponent<PositionC>(e);  // All entities have the Position component
        auto& col = GetComponent<CollisionC>(e); // All entities have the Position component

        for (const auto& tileObject : teleporters)
        {
            const Rectangle tileObjectRect = {tileObject.x, tileObject.y, tileObject.width, tileObject.height};
            assert(col.shape == Shape::RECT); // Only works when it's a rectangle
            const Rectangle entityRect = {pos.x, pos.y, col.p1, col.p2};
            if (CheckCollisionRecs(tileObjectRect, entityRect))
            {
                pos.map = MapID(tileObject.getClass());
                pos.x = 11 * 24;
                pos.y = 9 * 24;
            }
        }
    }
}