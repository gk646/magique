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

// Core
#include "core/Collision.h"
#include "core/Core.h"
#include "core/Draw.h"
#include "core/Game.h"
#include "core/Particles.h"
#include "core/Sound.h"
#include "core/Types.h"

// ECS
#include "ecs/Components.h"
#include "ecs/ECS.h"
#include "ecs/Scripting.h"

// Gamedev
#include "gamedev/Achievements.h"
#include "gamedev/Pathfinding.h"
#include "gamedev/CommandLine.h"
#include "gamedev/Procedural.h"
#include "gamedev/Localization.h"

// Multiplayer
#include "multiplayer/Multiplayer.h"
#include "multiplayer/LocalSockets.h"

// Persistence
#include "persistence/GameLoader.h"
#include "persistence/GameSaver.h"
#include "persistence/container/GameConfig.h"
#include "persistence/container/GameSave.h"

// Steam
#include "steam/Steam.h"

// UI
#include "ui/UI.h"
#include "ui/TextFormat.h"
#include "ui/types/UIContainer.h"

// Util
#include "util/Logging.h"
#include "util/Strings.h"
#include "util/Jobs.h"
#include "util/Compression.h"

#endif // MAGIQUE_H