#pragma once
#ifndef MAGIQUE_FWD_HPP
#define MAGIQUE_FWD_HPP

#include <cstdint> // My favourite header

namespace magique
{
    //----------------- CORE -----------------//

    struct Keybind; // Efficient representation of a keybind
    struct Setting; // A
    struct Puint;   // uint16_t point class

    //----------------- LOADING & SAVING -----------------//

    struct AssetLoader;    // Handles loading all major game files -  Start -> MainMenu
    struct AssetContainer; // Asset list loaded from a compiled asset pack

    struct GameLoader; // Handles loading individual world state and player save - MainMenu -> Game
    struct GameSaver;  // Handles saving the session and reseting the state  - Game -> MainMenu
    struct GameSave;   // The game save object

    struct ConfigLoader; // Handles loading the config
    struct ConfigSaver;  // Handles saving the config
    struct GameConfig;   // The config persistence object

    struct ExecutorI; // Task loading interface

    //-----------------LOGIC-----------------//

    struct DrawTickData; // Data for one draw tick - guaranteed to not change during the tick

    //-----------------ECS-----------------//

    struct EntityScript;
    enum EventType : uint8_t;

    //----------------- ASSETS -----------------//

    struct Asset;                 // Memory container for any asset
    struct TextureRegion;         // All textures are stitched into a atlas, you can only retrieve their region
    struct SpriteSheet;           // Identifies a sprite sheet
    enum AtlasType : uint8_t;     // Specifies which atlas to add the texture to
    enum class handle : uint32_t; // Resource handle
    struct TileMap;               // Defines which textures to draw at each tile with a number
    struct TileSet;               // Defines the details for all tiles in a project - collision ...
    struct TileSheet;             // Stores the textures for all tiles in a project

    //----------------- GRAPHICS -----------------//

    enum class LightingModel : uint8_t; // Raytracing, Static Shadows, None

    //----------------- Multiplayer -----------------//

    enum UpdateFlag : uint8_t;

    //----------------- UTIL -----------------//

    enum LogLevel : int;
    struct Scheduler;
    enum class jobHandle : uint16_t;
    struct IJob;

} // namespace magique

//----------------- USER DEFINED -----------------//

enum EntityID : uint16_t;             // User implemented to identify entity types
enum class HandleID : int;            // User implemented to identify handles
enum class MapID : uint8_t;           // User implemented to identify maps
enum class KeybindID : uint16_t;      // User implemented to identify keybinds
enum class SettingID : uint16_t;      // User implemented to identify settings
enum class ConfigStringID : uint16_t; // User implemented to identify config string
enum class StorageID : int;           // User implemented to identify stored information

//-----------------RAYLIB-----------------//

struct Camera2D;
struct Font;
struct Vector2;
struct Vector3;
struct Color;
struct Texture;
struct Sound;
struct Music;

#endif // MAGIQUE_FWD_HPP