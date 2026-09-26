#include "WizardQuest.h"
#include "loading/Loaders.h"
#include "ecs/Components.h"
#include "ecs/Scripts.h"

void TileLoader::execute(AssetPack& assets)
{
    GLOBAL.tilemaps[MapID::LEVEL_1] = ImportTileMap(assets["Level1.tmj"]);
    GLOBAL.tilemaps[MapID::LEVEL_2] = ImportTileMap(assets["Level2.tmj"]);
    GLOBAL.tilemaps[MapID::LOBBY] = ImportTileMap(assets["lobby.tmj"]);
    GLOBAL.tileset = ImportTileSet(assets["topdown.tmj"]);
}

void TextureLoader::execute(AssetPack& assets)
{
    GLOBAL.tileSheet = ImportTileSheet(assets["topdown.png"], 8, 3);

    // Load player animations - from single image files
    {
        Animation playerAnim{3}; // Scaling factor is 3 just like for the tilemap and collision
        std::vector<Asset> idle;
        std::vector<Asset> jump;
        std::vector<Asset> run;
        // Iterate all basic animation directory and sort them into vectors
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
        assets.forEachIn("characters/basic/basic/", func);

        // Register the animations
        const Point offset = {-5, -6};
        handle = ImportSpriteVec(idle, AtlasID::ENTITIES, 3);
        playerAnim.addAnimation(AnimationState::IDLE, GetSpriteSheet(handle), 12, offset);

        handle = RegisterSpriteSheetVec(jump, AtlasID::ENTITIES, 3);
        playerAnim.addAnimation(AnimationState::JUMP, GetSpriteSheet(handle), 6, offset);

        handle = RegisterSpriteSheetVec(run, AtlasID::ENTITIES, 3);
        playerAnim.addAnimation(AnimationState::RUN, GetSpriteSheet(handle), 6, offset);

        // Register the animation data for an entity type
        RegisterEntityAnimation(PLAYER, playerAnim);
        RegisterEntityAnimation(NET_PLAYER, playerAnim);
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

void EntityLoader::execute(AssetPack& res)
{
    ScriptingSetScript(EntityType::PLAYER, new PlayerScript());
    EntityRegister(EntityType::PLAYER,
                   [](entt::entity e, EntityType type)
                   {
                       ComponentGiveActor(e);
                       ComponentGiveCamera(e);
                       GiveCollisionRect(e, 20, 30);
                       ComponentGive<EntityStatsC>(e);
                       ComponentGive<MovementC>(e);
                       GiveAnimation(e, type, AnimationState::IDLE);
                   });

    ScriptingSetScript(TROLL, new TrollScript());
    EntityRegister(TROLL,
                   [](entt::entity e, EntityType type)
                   {
                       GiveCollisionRect(e, 20, 30);
                       ComponentGive<EntityStatsC>(e);
                       ComponentGive<MovementC>(e);
                       GiveAnimation(e, type, AnimationState::IDLE);
                   });

    ScriptingSetScript(NET_PLAYER, new NetPlayerScript());
    EntityRegister(NET_PLAYER,
                   [](entt::entity e, EntityType type)
                   {
                       ComponentGiveActor(e);
                       GiveCollisionRect(e, 20, 30);
                       ComponentGive<EntityStatsC>(e);
                       ComponentGive<MovementC>(e);
                       GiveAnimation(e, type, AnimationState::IDLE);
                   });
}
