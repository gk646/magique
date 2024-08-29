#ifndef LOADERS_H
#define LOADERS_H

#include <WizardQuest.h>
#include <magique/assets/AssetManager.h>
#include <magique/assets/HandleRegistry.h>
#include <magique/assets/container/AssetContainer.h>
#include <magique/core/Types.h>

struct TileLoader final : ITask<AssetContainer>
{
    void execute(AssetContainer& res) override
    {
        auto handle = RegisterTileMap(res.getAsset("Level1.tmx"));
        RegisterHandle(handle, HandleID::LEVEL_1);
        handle = RegisterTileMap(res.getAsset("Level2.tmx"));
        RegisterHandle(handle, HandleID::LEVEL_2);
        handle = RegisterTileMap(res.getAsset("Level3.tmx"));
        RegisterHandle(handle, HandleID::LEVEL_3);
        handle = RegisterTileMap(res.getAsset("Level4.tmx"));
        RegisterHandle(handle, HandleID::LEVEL_4);

        // Load TileSet
        handle = RegisterTileSet(res.getAsset("Dungeon.tsx"));
        RegisterHandle(handle, HandleID::TILE_SET);
    }
};

struct TextureLoader final : ITask<AssetContainer>
{
    void execute(AssetContainer& res) override
    {
        auto handle = RegisterTileSheet(res.getAsset("Dungeon_Tileset.png"),16,3);
        RegisterHandle(handle, HandleID::TILESHEET);
    }
};

struct EntityLoader final : ITask<AssetContainer>
{
    void execute(AssetContainer &res) override
    {
        SetScript(PLAYER, new PlayerScript());

        RegisterEntity(PLAYER,
                       [](entt::entity e)
                       {
                           GiveActor(e);
                           GiveCamera(e);
                           GiveCollisionRect(e, 20, 30);
                           GiveComponent<EntityStatsC>(e);
                           GiveScript(e);
                           GiveComponent<MovementC>(e);
                       });
    }
};

#endif // LOADERS_H