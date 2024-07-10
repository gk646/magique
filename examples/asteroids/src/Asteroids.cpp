#include "Asteroids.h"

#include <raylib/raylib.h>

#include <magique/assets/AssetLoader.h>
#include <magique/assets/AssetManager.h>
#include <magique/assets/HandleRegistry.h>
#include <magique/core/Core.h>
#include <magique/ecs/ECS.h>

void Asteroids::onStartup(magique::AssetLoader &al, magique::GameConfig &config)
{
    // Set FPS to 120
    SetTargetFPS(120);

    // Load the sounds on a background thread
    al.registerTask(
        [](magique::AssetContainer &assets)
        {
            // Iterate the sfx directory for the sounds
            assets.iterateDirectory("SFX",
                                    [](const magique::Asset &asset)
                                    {
                                        // Register the sound with the asset manager
                                        auto handle = RegisterSound(asset);
                                        // Register the handle to the sound with its direct filename - without extension
                                        RegisterHandle(handle, asset.getFileName(false));
                                    });
        },
        magique::BACKGROUND_THREAD, magique::MEDIUM, 8);

    // Load the texture on the main thread as they require gpu acess
    al.registerTask(
        [](magique::AssetContainer &assets)
        {
            // Iterate the sprites directory for the textures
            assets.iterateDirectory("SPRITES",
                                    [](const magique::Asset &asset)
                                    {
                                        magique::handle handle;
                                        // Correctly handle the sprite sheet
                                        if (TextIsEqual(asset.getFileName(), "EXPLOSION SPRITE SHEET.png"))
                                        {
                                            handle = RegisterSpritesheet(asset, 20, 20);
                                        }
                                        else
                                        {
                                            handle = RegisterTexture(asset);
                                        }
                                        // Register the handle to the texture with its filename - without extension
                                        RegisterHandle(handle, asset.getFileName(false));
                                    });
        },
        magique::MAIN_THREAD, magique::MEDIUM, 2);

    // Set the player script
    SetScript(PLAYER, new PlayerScript());

    // Register the player entity
    magique::RegisterEntity(EntityID::PLAYER,
                            [](entt::registry &registry, entt::entity entity)
                            {
                                magique::GiveCamera(entity);
                                magique::GiveActor(entity);
                                magique::GiveCollision(entity, magique::RECT, 12, 12, 6, 6);
                                auto& anim = registry.emplace<AnimationC<>>(entity);
                                anim.
                            });

    // Create a player
    magique::CreateEntity(PLAYER, 0, 0, MapID::LEVEL_1);
}

void Asteroids::onCloseEvent() { shutDown(); }

void Asteroids::updateGame(entt::registry &registry) {}


void Asteroids::drawWorld(Camera2D &camera) {}

void Asteroids::drawGame(entt::registry &registry, Camera2D &camera)
{
    auto &drawEntities = magique::GetDrawEntities();
    auto spriteEntities = magique::REGISTRY.view<>()
    for(auto e : drawEntities)
    {

    }
}

void Asteroids::drawUI() {}

void PlayerScript::onKeyEvent(entt::registry &registry, entt::entity me)
{
    auto &pos = registry.get<PositionC>(me);
    if (IsKeyDown(KEY_W))
        pos.y -= 5;
    if (IsKeyDown(KEY_S))
        pos.y += 5;
    if (IsKeyDown(KEY_A))
        pos.x -= 5;
    if (IsKeyDown(KEY_D))
        pos.x += 5;
}