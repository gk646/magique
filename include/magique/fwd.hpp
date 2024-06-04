#pragma once
#ifndef MAGIQUE_FWD_HPP
#define MAGIQUE_FWD_HPP

#include <cstdint> // My favourite header

//----------------- LOADING -----------------//
namespace magique
{
    struct GameLoader;     // Handles loading all major game files -  Start -> MainMenu
    struct SaveLoader;     // Handles loading individual world state and player save - MainMenu -> Game
    struct SaveUnloader;   // Handles saving the session and reseting the state  - Game -> MainMenu
    struct AssetContainer; // Asset List loaded from a compiled asset pack

    //-----------------LEVELS-----------------//
    struct Map; // Holds information about the current level


    //-----------------LOGIC-----------------//
    struct DrawTickData; // Data for one draw tick - guaranteed to not change during the tick

    //-----------------ECS-----------------//


    //----------------- ASSETS -----------------//
    struct Asset;         // Memory container for any asset
    struct TextureRegion; // All textures are stitched into a atlas, you can only retrieve their region
    struct SpriteSheet;
    enum AtlasType : uint8_t;      // Specifies which atlas to add the texture to
    enum class Resource : uint8_t; // Resource type specifier
} // namespace magique

enum class EntityType : uint16_t;


//-----------------RAYLIB-----------------//

struct Camera2D;
struct Font;
struct Vector2;
struct Vector3;
struct Color;
struct Texture;
struct Sound;
struct Music;

//-----------------DATA STRUCTURES-----------------//

template <typename T>
struct fast_vector; // Library vector type


namespace magique
{
    template <typename T>
    using vector = fast_vector<T>;

    // Cant forward declare that easily
    //template <typename K, typename V, typename Hash>
    // using HashMap = tsl::robin_map<K, V, Hash>;

} // namespace magique


#endif // MAGIQUE_FWD_HPP