#ifndef LOADERS_H
#define LOADERS_H

#include <WizardQuest.h>
#include <magique/assets/AssetManager.h>
#include <magique/assets/HandleRegistry.h>
#include <magique/assets/container/AssetContainer.h>
#include <magique/core/Animations.h>
#include <magique/core/Types.h>

struct TileLoader final : ITask<AssetContainer>
{
    void execute(AssetContainer& res) override
    {
        auto handle = RegisterTileMap(res.getAsset("Level1.tmx"));
        RegisterHandle(handle, HandleID::LEVEL_1);

        // Load TileSet
        handle = RegisterTileSet(res.getAsset("topdown.tsx"));
        RegisterHandle(handle, HandleID::TILE_SET);
    }
};

struct TextureLoader final : ITask<AssetContainer>
{
    void execute(AssetContainer& assets) override
    {
        auto handle = RegisterTileSheet(assets.getAsset("topdown.png"), 8, 3);
        RegisterHandle(handle, HandleID::TILESHEET);

        // Load player animations - from single image files
        EntityAnimation playerAnim;
        std::vector<Asset> idle;
        std::vector<Asset> jump;
        std::vector<Asset> run;
        auto func = [&](Asset asset)
        {
            if (asset.contains("jump"))
            {
                jump.push_back(asset);
            }
            else if (asset.contains("idle"))
            {
                idle.push_back(asset);
            }
            else if (asset.contains("run"))
            {
                run.push_back(asset);
            }
        };
        assets.iterateDirectory("characters/basic/basic/", func);

        handle = RegisterSpriteSheetVec(idle, AtlasID::ENTITIES, 3);
        playerAnim.addAnimation(AnimationState::IDLE, GetSpriteSheet(handle), 12);

        handle = RegisterSpriteSheetVec(jump, AtlasID::ENTITIES, 3);
        playerAnim.addAnimation(AnimationState::JUMP, GetSpriteSheet(handle), 6);

        handle = RegisterSpriteSheetVec(run, AtlasID::ENTITIES, 3);
        playerAnim.addAnimation(AnimationState::RUN, GetSpriteSheet(handle), 6);

        RegisterEntityAnimation(PLAYER, playerAnim);
    }
};

struct EntityLoader final : ITask<AssetContainer>
{
    void execute(AssetContainer& res) override
    {
        SetEntityScript(PLAYER, new PlayerScript());
        RegisterEntity(PLAYER,
                       [](entt::entity e, EntityType type)
                       {
                           GiveActor(e);
                           GiveCamera(e);
                           GiveCollisionRect(e, 20, 30);
                           GiveComponent<EntityStatsC>(e);
                           GiveScript(e);
                           GiveComponent<MovementC>(e);
                           GiveAnimation(e, type, AnimationState::IDLE);
                       });
    }
};

#endif // LOADERS_H