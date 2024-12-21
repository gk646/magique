// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_FWD_HPP
#define MAGIQUE_FWD_HPP

#include <cstdint> // My favourite header
#include <magique/config.h>

//================= USER DEFINED =================//

enum class GameState : int;          // Identifies different game states
enum EntityType : uint16_t;          // Identifies entity types       - uses uint16_t to be small for networking
enum class MapID : uint8_t;          // Identifies maps               - uses uint8_t  to be small for networking
enum class HandleID : int;           // Identifies handles
enum class ConfigID : int;           // Identifies stored config data
enum class StorageID : int;          // Identifies stored game save data
enum class MessageType : uint8_t;    // Identifies different network messages - uses uint8_t  to be small for networking
enum class AnimationState : uint8_t; // Identifies different animation states - uses uint8_t to be small for the ECS
enum class AtlasID : int;            // Identifies different texture atlas

namespace magique
{
    //================= CORE =================//
    struct Keybind;          // Efficient representation of a keybind
    struct Game;             // Core Game class
    struct ScreenEmitter;    // Particle emitter for screen (simple) particles
    struct EntityEmitter;    // Particle emitter for entity (complex) particles
    struct EntityAnimation;  // Shared animation data for a specific entity
    enum class LightingMode; // Raytracing, Static Shadows, None
    struct EmitterData;      // Data for particle emitters

    //================= LOADING & SAVING =================//
    struct AssetLoader;    // Handles loading all major game files -  Start -> MainMenu
    struct AssetContainer; // Asset list loaded from a compiled asset pack

    struct TaskInterface; // Handles loading individual world state and player save - MainMenu -> Game
    struct GameSaver;  // Handles saving the session and resetting the state  - Game -> MainMenu
    struct GameSaveData;   // The game save object

    struct ConfigLoader; // Handles loading the config
    struct ConfigSaver;  // Handles saving the config
    struct GameConfig;   // The config persistence object

    struct IExecutor; // Task loading interface
    namespace internal
    {
        template <typename T>
        struct TaskExecutor;
    }

    template <typename... Columns>
    struct DataTable;         // Type-safe database-like interface to save game data

    //=================ECS=================//
    struct EntityScript;
    enum EventType : uint8_t;
    enum CollisionLayer : uint8_t;
    enum class Shape : uint8_t;
    struct PositionC;  // Implicit position component
    struct CollisionC; // Built in collision component

    //================= ASSETS =================//
    struct Asset;                 // Memory container for any asset
    struct TextureRegion;         // All textures are stitched into an atlas, you can only retrieve their region
    struct SpriteSheet;           // Identifies a sprite sheet
    enum class handle : uint32_t; // Resource handle
    struct TileMap;               // Defines which textures to draw at each tile with a number
    struct TileSet;               // Defines the details for all tiles in a project - collision ...
    struct TileSheet;             // Stores the textures (id's) for all tiles in a project
    struct Playlist;              // Stores multiple music objects to easily play and loop them
    struct TileObject;            // TileObject imported from the Tile Editor
    struct CSVImport;             // CSV file import data

    //================= GAMEDEV =================//
    template <typename CustomData = int>
    struct BehaviorTree;
    template <typename CustomData = int>
    struct TreeBuilder;
    struct Quest;
    struct QuestNode;
    struct Achievement;
    struct ShareCodeFormat;
    struct ShareCodeData;
    struct ShareCode;
    struct ShareCodeProperty;
    struct Command;
    struct ConsoleData;
    struct ParamParser;
    struct ConsoleHandler;

    //================= Multiplayer =================//


    //================= UI =================//
    struct UIObject;
    struct UIContainer;
    struct UIData; // Internal
    struct LoadingScreen;
    enum class AnchorPosition : int;
    enum class UILayer : uint8_t;
    struct UIScene;
    struct Window;
    struct WindowManager;
    struct WindowButton;

    //================= UTIL =================//
    enum LogLevel : int;
    struct Scheduler;
    enum class jobHandle : uint16_t;
    struct IJob;

    //================= INTERNAL =================//
    struct AudioPlayer;

} // namespace magique


//=================RAYLIB=================//
struct Camera2D;
struct Font;
struct Vector2;
struct Vector3;
struct Color;
struct Texture;
struct Sound;
struct Music;
struct Rectangle;

#define MOUSE_LEFT_BUTTON 0
//=================RAYLIB=================//

namespace entt
{
    enum class entity : std::uint32_t;
}

#endif