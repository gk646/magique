#pragma once
#ifndef MAGIQUE_FWD_HPP
#define MAGIQUE_FWD_HPP

//-----------------LOADING-----------------//

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
} // namespace magique


struct Camera2D; // raylib


//-----------------DATA STRUCTURES-----------------//

template <typename T>
struct fast_vector; // Library vector type

namespace tsl
{
    template <class Key, class T, class Hash, class KeyEqual, class Allocator, bool StoreHash, class GrowthPolicy>
    class robin_map;
}


namespace magique
{
    template <typename T>
    using vector = fast_vector<T>;

    // Cant forward declare that easily
    //template <typename K, typename V, typename Hash>
    // using HashMap = tsl::robin_map<K, V, Hash>;

} // namespace magique


#endif // MAGIQUE_FWD_HPP