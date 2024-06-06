#pragma once
#ifndef MAGIQUE_FWD_HPP
#define MAGIQUE_FWD_HPP

#include <cstdint> // My favourite header

namespace magique
{
    //----------------- LOADING -----------------//

    struct GameLoader;     // Handles loading all major game files -  Start -> MainMenu
    struct SaveLoader;     // Handles loading individual world state and player save - MainMenu -> Game
    struct SaveUnloader;   // Handles saving the session and reseting the state  - Game -> MainMenu
    struct AssetContainer; // Asset list loaded from a compiled asset pack

    //-----------------LEVELS-----------------//
    struct Map; // Holds information about the current level

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

} // namespace magique

//----------------- USER DEFINED -----------------//

enum EntityID : uint16_t;   // User implemented to denote different entity types
enum class HandleID : int;  // User implemented to denote different handles
enum class MapID : uint8_t; // User implemented to denote different handles


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