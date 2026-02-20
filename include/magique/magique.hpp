// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_H
#define MAGIQUE_H

// The single include header

#include "fwd.hpp"

// Assets
#include "assets/AssetContainer.h"
#include "assets/AssetImport.h"
#include "assets/AssetLoader.h"
#include "assets/AssetPacker.h"
#include "assets/types/Playlist.h"
#include "assets/types/TileMap.h"
#include "assets/types/TileSet.h"
#include "assets/types/TileSheet.h"
#include "assets/types/TextLines.h"

// Core
#include "core/Animation.h"
#include "core/Collision.h"
#include "core/Engine.h"
#include "core/EngineUtil.h"
#include "core/Camera.h"
#include "core/Draw.h"
#include "core/Game.h"
#include "core/GameConfig.h"
#include "core/Particles.h"
#include "core/Sound.h"
#include "core/StaticCollision.h"
#include "core/Types.h"

// ECS
#include "ecs/Components.h"
#include "ecs/ECS.h"
#include "ecs/Scripting.h"
#include "ecs/GameSystem.h"

// Gamedev
#include "gamedev/Achievements.h"
#include "gamedev/AIAgent.h"
#include "gamedev/Console.h"
#include "gamedev/Localization.h"
#include "gamedev/PathFinding.h"
#include "gamedev/Procedural.h"
#include "gamedev/ShareCode.h"
#include "gamedev/VirtualClock.h"
#include "gamedev/UsefulStuff.h"

// Networking
#if defined( MAGIQUE_LAN) || defined(MAGIQUE_STEAM)
#include "multiplayer/Networking.h"
#include "multiplayer/LocalSocket.h"
#include "multiplayer/Lobby.h"
#endif

// Persistence
#include "persistence/GameSave.h"
#include "persistence/SaveLoader.h"

// Steam
#ifdef MAGIQUE_STEAM
#include "steam/Steam.h"
#include "steam/Matchmaking.h"
#include "steam/GlobalSocket.h"
#endif

// UI
#include "ui/UI.h"
#include "ui/TextFormat.h"
#include "ui/UIContainer.h"
#include "ui/controls/Button.h"
#include "ui/controls/Window.h"
#include "ui/controls/TextField.h"
#include "ui/controls/Slider.h"
#include "ui/controls/Checkbox.h"
#include "ui/controls/ListMenu.h"
#include "ui/controls/ScrollPane.h"
#include "ui/controls/DropDown.h"

// Util
#include "util/Logging.h"
#include "util/Strings.h"
#include "util/JobSystem.h"
#include "util/Compression.h"
#include "util/RayUtils.h"
#include "util/Math.h"

#endif // MAGIQUE_H
