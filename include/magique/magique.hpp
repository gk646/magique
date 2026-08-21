// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_H
#define MAGIQUE_H

// The single include header

#include "fwd.hpp"

// Assets
#include "assets/AssetPack.h"
#include "assets/AssetImport.h"
#include "assets/AssetLoader.h"
#include "assets/AssetPacker.h"
#include "assets/JSON.h"
#include "assets/types/Playlist.h"
#include "assets/types/TileMap.h"
#include "assets/types/TileSet.h"
#include "assets/types/TileSheet.h"
#include "assets/types/TextLines.h"

// Core
#include "core/Camera.h"
#include "core/Collision.h"
#include "core/Console.h"
#include "core/Engine.h"
#include "core/EngineUtil.h"
#include "core/Game.h"
#include "core/Sound.h"
#include "core/StaticCollision.h"
#include "core/Types.h"

// ECS
#include "ecs/Components.h"
#include "ecs/ECS.h"
#include "ecs/GameSystem.h"
#include "ecs/Scripting.h"

// Gamedev
#include "gamedev/Achievements.h"
#include "gamedev/ActionInput.h"
#include "gamedev/Events.h"
#include "gamedev/Localization.h"
#include "gamedev/PathFinding.h"
#include "gamedev/Procedural.h"
#include "gamedev/ShareCode.h"
#include "gamedev/TextFormat.h"
#include "gamedev/Tweens.h"
#include "gamedev/VirtualClock.h"
#include "gamedev/UsefulStuff.h"

// Graphics
#include "graphics/Animation.h"
#include "graphics/BaseShaders.h"
#include "graphics/Draw.h"
#include "graphics/Lighting.h"
#include "graphics/Particles.h"
#include "graphics/TextDrawer.h"

// Networking
#if defined(MAGIQUE_LAN) || defined(MAGIQUE_STEAM)
#include "networking/Networking.h"
#include "networking/LocalSocket.h"
#include "networking/Lobby.h"
#endif

// Persistence
#include "persist/GameStorage.h"

// Steam
#ifdef MAGIQUE_STEAM
#include "steam/Steam.h"
#include "steam/Matchmaking.h"
#include "steam/GlobalSocket.h"
#endif

// UI
#include "ui/UI.h"
#include "ui/UIObject.h"
#include "ui/UIContainer.h"
#include "ui/Scene.h"
#include "ui/WindowManager.h"
#include "ui/controls/Button.h"
#include "ui/controls/Window.h"
#include "ui/controls/TextField.h"
#include "ui/controls/Slider.h"
#include "ui/controls/Checkbox.h"
#include "ui/controls/ListChooser.h"
#include "ui/controls/GridChooser.h"
#include "ui/controls/ScrollPane.h"
#include "ui/controls/DropDown.h"

// Util
#include "util/Logging.h"
#include "util/Strings.h"
#include "util/JobSystem.h"
#include "util/RayUtils.h"
#include "util/Math.h"
#include "util/Datastructures.h"

#endif // MAGIQUE_H
