#ifndef FWD_HPP
#define FWD_HPP

struct GameLoader;   // Handles loading all major game files -  Start -> MainMenu
struct SaveLoader;   // Handles loading individual world state and player save - MainMenu -> Game
struct SaveUnloader; // Handles saving the session and reseting the state  - Game -> MainMenu


struct AssetContainer; // Asset List loaded from a compiled asset pack


struct Camera2D; // raylib
#endif           // FWD_HPP