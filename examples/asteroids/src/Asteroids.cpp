#include "Asteroids.h"

#include <raylib/raylib.h>

#include <magique/assets/AssetLoader.h>
#include <magique/assets/AssetManager.h>
#include <magique/assets/HandleRegistry.h>
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


    // Register the player entity
    magique::RegisterEntity(EntityID::PLAYER, [](entt::registry &registry, entt::entity entity) {});
}

void Asteroids::onCloseEvent() { shutDown(); }

void Asteroids::updateGame(entt::registry &registry) {}

void Asteroids::drawWorld(Camera2D &camera) {}

void Asteroids::drawGame(entt::registry &registry, Camera2D &camera) {}

void Asteroids::drawUI() {}