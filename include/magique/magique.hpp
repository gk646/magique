// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_H
#define MAGIQUE_H

// The single include header

#include "fwd.hpp"

// Assets
#include "assets/AssetPacker.h"
#include "assets/AssetManager.h"
#include "assets/AssetLoader.h"
#include "assets/HandleRegistry.h"
#include "assets/container/AssetContainer.h"
#include "assets/types/Playlist.h"
#include "assets/types/TileMap.h"
#include "assets/types/TileSet.h"
#include "assets/types/TileSheet.h"

// Core
#include "core/Animations.h"
#include "core/CollisionDetection.h"
#include "core/Core.h"
#include "core/Debug.h"
#include "core/Draw.h"
#include "core/Game.h"
#include "core/Particles.h"
#include "core/Sound.h"
#include "core/StaticCollision.h"
#include "core/Types.h"

// ECS
#include "ecs/Components.h"
#include "ecs/ECS.h"
#include "ecs/Scripting.h"

// Gamedev
#include "gamedev/Achievements.h"
#include "gamedev/AIAgent.h"
#include "gamedev/Console.h"
#include "gamedev/Localization.h"
#include "gamedev/PathFinding.h"
#include "gamedev/Procedural.h"
#include "gamedev/ShareCode.h"
#include "gamedev/VirtualClock.h"

// Multiplayer
#ifdef MAGIQUE_LAN
#include "multiplayer/Multiplayer.h"
#include "multiplayer/LocalSockets.h"
#endif

// Persistence
#include "persistence/GameLoader.h"
#include "persistence/GameSaver.h"
#include "persistence/container/GameConfig.h"
#include "persistence/container/GameSave.h"

// Steam
#ifdef MAGIQUE_STEAM
#include "steam/Steam.h"
#include "steam/Lobbies.h"
#include "steam/GlobalSockets.h"
#include "multiplayer/Multiplayer.h"
#include "multiplayer/LocalSockets.h"
#endif

// UI
#include "ui/UI.h"
#include "ui/TextFormat.h"
#include "ui/types/UIContainer.h"
#include "ui/controls/Button.h"
#include "ui/controls/Window.h"
#include "ui/controls/TextField.h"

// Util
#include "util/Logging.h"
#include "util/Strings.h"
#include "util/JobSystem.h"
#include "util/Compression.h"
#include "util/RayUtils.h"
#include "util/Math.h"

#endif // MAGIQUE_H