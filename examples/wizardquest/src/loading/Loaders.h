#ifndef LOADERS_H
#define LOADERS_H

#include <WizardQuest.h>
#include <ecs/Scripts.h>
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

        handle = RegisterTileMap(res.getAsset("Level2.tmx"));
        RegisterHandle(handle, HandleID::LEVEL_2);

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
        {
            EntityAnimation playerAnim{3}; // Scaling factor is 3 just like for the tilemap and collision
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

            const Point offset = {-5, -6};
            handle = RegisterSpriteSheetVec(idle, AtlasID::ENTITIES, 3);
            playerAnim.addAnimation(AnimationState::IDLE, GetSpriteSheet(handle), 12, offset);

            handle = RegisterSpriteSheetVec(jump, AtlasID::ENTITIES, 3);
            playerAnim.addAnimation(AnimationState::JUMP, GetSpriteSheet(handle), 6, offset);

            handle = RegisterSpriteSheetVec(run, AtlasID::ENTITIES, 3);
            playerAnim.addAnimation(AnimationState::RUN, GetSpriteSheet(handle), 6, offset);

            RegisterEntityAnimation(PLAYER, playerAnim);
        }

        {
            EntityAnimation trollAnim{3}; // Scaling factor is 3 just like for the tilemap and collision
            std::vector<Asset> idle;
            std::vector<Asset> attack1;
            std::vector<Asset> attack2;
            std::vector<Asset> run;
            auto func = [&](Asset asset)
            {
                if (asset.contains("idle"))
                {
                    idle.push_back(asset);
                }
                else if (asset.contains("attack1"))
                {
                    attack1.push_back(asset);
                }
                else if (asset.contains("attack2"))
                {
                    attack2.push_back(asset);
                }
                else if (asset.contains("run"))
                {
                    run.push_back(asset);
                }
            };
            assets.iterateDirectory("characters/troll/", func);

            const Point offset = {-6, -6};
            handle = RegisterSpriteSheetVec(idle, AtlasID::ENTITIES, 3);
            trollAnim.addAnimation(AnimationState::IDLE, GetSpriteSheet(handle), 12, offset);

            handle = RegisterSpriteSheetVec(attack1, AtlasID::ENTITIES, 3);
            trollAnim.addAnimation(AnimationState::ATTACK_1, GetSpriteSheet(handle), 6, offset);

            handle = RegisterSpriteSheetVec(attack2, AtlasID::ENTITIES, 3);
            trollAnim.addAnimation(AnimationState::ATTACK_2, GetSpriteSheet(handle), 6, offset);

            handle = RegisterSpriteSheetVec(run, AtlasID::ENTITIES, 3);
            trollAnim.addAnimation(AnimationState::RUN, GetSpriteSheet(handle), 6, offset);

            RegisterEntityAnimation(TROLL, trollAnim);
        }
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

        SetEntityScript(TROLL, new TrollScript());
        RegisterEntity(TROLL,
                       [](entt::entity e, EntityType type)
                       {
                           GiveCollisionRect(e, 20, 30);
                           GiveComponent<EntityStatsC>(e);
                           GiveScript(e);
                           GiveComponent<MovementC>(e);
                           GiveAnimation(e, type, AnimationState::IDLE);
                       });
    }
};

#endif // LOADERS_H