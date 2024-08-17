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

#endif // LOADERS_H