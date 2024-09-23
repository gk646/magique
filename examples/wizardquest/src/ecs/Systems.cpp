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
            if (objectName == "MapTeleporter")
            {
                Teleporter teleporter;
                teleporter.x = 3 * tileObject.x;
                teleporter.y = 3 * tileObject.y;
                teleporter.w = 3 * tileObject.width;
                teleporter.h = 3 * tileObject.height;
                teleporter.origin = MapID(i);
                teleporter.destination = MapID(tileObject.getID());
                if (teleporter.destination == MapID::LEVEL_1) // Statically decide
                {
                    teleporter.outX = 11 * 24;
                    teleporter.outY = 9 * 24;
                }
                teleporters.push_back(teleporter);
            }
            else if (objectName == "Teleporter")
            {
                Teleporter teleporter{};
                teleporter.origin = MapID(i);
                teleporter.destination = MapID(i);
                teleporter.x = 3 * tileObject.x;
                teleporter.y = 3 * tileObject.y;
                teleporter.w = 3 * tileObject.width;
                teleporter.h = 3 * tileObject.height;
                teleporter.outX = 24 * tileObject.customProperties[0].getInt();
                teleporter.outY = 24 * tileObject.customProperties[1].getInt();
                teleporters.push_back(teleporter);
            }
        }
    }
}

void TeleportSystem::update()
{
    for (const auto e : GetUpdateEntities())
    {
        if (!EntityHasComponents<CollisionC, MovementC>(e))
            continue;
        auto& pos = GetComponent<PositionC>(e); // All entities have the Position component
        auto& col = GetComponent<CollisionC>(e);
        auto& mov = GetComponent<MovementC>(e);

        if (mov.teleportCooldownCounter < MovementC::TELEPORT_COOLDOWN)
            continue;

        for (const auto& teleporter : teleporters)
        {
            if (pos.map != teleporter.origin)
                continue;

            const Rectangle tileObjectRect = {teleporter.x, teleporter.y, teleporter.w, teleporter.h};
            assert(col.shape == Shape::RECT); // Only works when it's a rectangle
            const Rectangle entityRect = {pos.x, pos.y, col.p1, col.p2};
            if (CheckCollisionRecs(tileObjectRect, entityRect))
            {
                pos.map = teleporter.destination;
                pos.x = teleporter.outX;
                pos.y = teleporter.outY;
                mov.teleportCooldownCounter = 0;
                break;
            }
        }
    }
}