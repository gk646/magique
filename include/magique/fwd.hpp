#ifndef MAGIQUE_FWD_HPP
#define MAGIQUE_FWD_HPP

#include <cstdint> // My favourite header

//----------------- USER DEFINED -----------------//

enum class GameState : int;       // Identifies different game states
enum EntityID : uint16_t;         // Identifies entity types - uses uint16_t to be small for networking
enum class MapID : uint8_t;       // Identifies maps         - uses uint8_t  to be small for networking
enum class HandleID : int;        // Identifies handles
enum class ConfigID : int;        // Identifies stored config data
enum class StorageID : int;       // Identifies stored game save data
enum class MessageType : uint8_t; // Identifies different network messages

namespace magique
{
    //----------------- CORE -----------------//
    struct Keybind;       // Efficient representation of a keybind
    struct Game;          // Core Game class
    struct ScreenEmitter; // Particle emitter for screen (simple) particles
    struct EntityEmitter; // Particle emitter for entity (complex) particles

    //----------------- LOADING & SAVING -----------------//
    struct AssetLoader;    // Handles loading all major game files -  Start -> MainMenu
    struct AssetContainer; // Asset list loaded from a compiled asset pack

    struct GameLoader; // Handles loading individual world state and player save - MainMenu -> Game
    struct GameSaver;  // Handles saving the session and reseting the state  - Game -> MainMenu
    struct GameSave;   // The game save object

    struct ConfigLoader; // Handles loading the config
    struct ConfigSaver;  // Handles saving the config
    struct GameConfig;   // The config persistence object

    struct IExecutor; // Task loading interface

    template <typename... Columns>
    struct DataTable; // Type-safe database-like interface to save game data

    //-----------------ECS-----------------//
    struct EntityScript;
    enum EventType : uint8_t;
    enum CollisionLayer : uint8_t;
    enum class Shape : uint8_t;

    //----------------- ASSETS -----------------//
    struct Asset;                 // Memory container for any asset
    struct TextureRegion;         // All textures are stitched into a atlas, you can only retrieve their region
    struct SpriteSheet;           // Identifies a sprite sheet
    enum AtlasType : uint8_t;     // Specifies which atlas to add the texture to
    enum class handle : uint32_t; // Resource handle
    struct TileMap;               // Defines which textures to draw at each tile with a number
    struct TileSet;               // Defines the details for all tiles in a project - collision ...
    struct TileSheet;             // Stores the textures (id's) for all tiles in a project
    struct Playlist;              // Stores mulitple music objects to easily play and loop them

    //----------------- GRAPHICS -----------------//
    enum class LightingMode; // Raytracing, Static Shadows, None
    struct EmitterData;      // Data for particle emitters

    //----------------- Multiplayer -----------------//

    //----------------- GAMEDEV -----------------//
    template <typename CustomData = int>
    struct BehaviorTree;
    template <typename CustomData = int>
    struct TreeBuilder;
    struct Quest;
    struct QuestNode;
    struct Achievement;

    //----------------- UI -----------------//
    struct UIObject;
    struct UIContainer;
    struct UIData; // Internal
    struct LoadingScreen;
    enum class AnchorPosition : int;
    enum class UILayer : uint8_t;

    //----------------- UTIL -----------------//
    enum LogLevel : int;
    struct Scheduler;
    enum class jobHandle : uint16_t;
    struct IJob;

    //----------------- INTERNAL -----------------//
    struct AudioPlayer;

} // namespace magique


//-----------------RAYLIB-----------------//
struct Camera2D;
struct Font;
struct Vector2;
struct Vector3;
struct Color;
struct Texture;
struct Sound;
struct Music;
struct Rectangle;

//----------------- STD -----------------//

// Feel free to comment this out and #include <functional> - worked fine for me
namespace std
{
    template <class>
    class function;
} // namespace std


#endif