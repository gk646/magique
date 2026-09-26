#include "WizardQuest.h"
#include "ecs/Systems.h"
#include "ecs/Components.h"

void MovementSystem::update()
{
    for (const auto e : EngineGetUpdateEntities())
    {
        if (!EntityHasAll<PositionC, MovementC>(e))
            continue;
        auto& pos = ComponentGet<PositionC>(e);
        auto& move = ComponentGet<MovementC>(e);
        pos.pos += move.getVelocity();
    }
}

void AnimationSystem::update()
{
    for (const auto e : EngineGetUpdateEntities())
    {
        if (EntityHasAll<AnimationC>(e))
            ComponentGet<AnimationC>(e).update();
    }
}

void TeleportSystem::setup()
{
    for (auto map : EnumValues<MapID>())
    {
        auto& tileMap = GLOBAL.tilemaps[map];
        if (tileMap.getObjectLayers().empty())
            continue;

        for (const auto& tileObject : tileMap.getObjectLayers().front())
        {
            std::string objectName = std::string(tileObject.getName());
            if (objectName == "MapTeleporter")
            {
                Teleporter teleporter;
                teleporter.bounds = tileObject.bounds * 3;
                teleporter.origin = map;
                teleporter.destination = tileObject.getEnumProperty<MapID>("target").value_or(MapID::LOBBY);
                if (teleporter.destination == MapID::LEVEL_1) // Statically decide
                {
                    teleporter.target = Point{11, 9} * 24;
                }
                teleporters.push_back(teleporter);
            }
            else if (objectName == "Teleporter")
            {
                Teleporter teleporter{};
                teleporter.origin = map;
                teleporter.destination = map;
                teleporter.bounds = tileObject.bounds * 3;
                teleporter.target.x = tileObject.getIntProperty("targetX").value_or(0) * 24;
                teleporter.target.y = tileObject.getIntProperty("targetY").value_or(0) * 24;
                teleporters.push_back(teleporter);
            }
        }
    }
    for (int i = 0; i < (int)MapID::MAPS_END; ++i)
    {
    }
}

void TeleportSystem::update()
{
    for (const auto e : EngineGetUpdateEntities())
    {
        if (!EntityHasAll<CollisionC, MovementC>(e))
            continue;
        auto& pos = ComponentGet<PositionC>(e); // All entities have the Position component
        auto& mov = ComponentGet<MovementC>(e);

        if (mov.teleportCooldownCounter < MovementC::TELEPORT_COOLDOWN)
            continue;

        for (const auto& teleporter : teleporters)
        {
            if (pos.map != teleporter.origin)
                continue;

            if (CheckCollisionEntityRect(e, teleporter.bounds))
            {
                pos.map = teleporter.destination;
                pos.pos = teleporter.target;
                mov.teleportCooldownCounter = 0;
                break;
            }
        }
    }
}
