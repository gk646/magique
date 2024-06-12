#ifndef MAGIQUE_TYPES_H
#define MAGIQUE_TYPES_H

#include "magique/fwd.hpp"

//-----------------------------------------------
// Types Modules
//-----------------------------------------------
// ................................................................................
// These are common types used by the engine
// ................................................................................

namespace magique
{
    struct TextureRegion final // All textures are part of an atlas and can not be referenced as standalone
    {
        int16_t offX;   // Horizontal offset from the top left of the atlas
        int16_t offY;   // Vertical offset from the top left of the atlas
        int16_t width;  // Width of the texture
        int16_t height; // Height of the texture
        uint16_t id;    // The texture id
    };

    struct SpriteSheet final
    {
        int16_t offX;    // Horizontal offset from the top left of the atlas of the first frame
        int16_t offY;    // Vertical offset from the top left of the atlas of the first frame
        int16_t width;   // Width of a frame
        int16_t height;  // Height of a frame
        uint16_t id;     // The texture id
        uint16_t frames; // Total number of frames
    };

    enum class LightingModel : uint8_t
    {
        STATIC_SHADOWS, // Default
        RAY_TRACING,
        NONE,
    };

    // Efficient representation of a keybind with optional modifiers
    struct Keybind final
    {
        Keybind() = default;
        explicit Keybind(int keyCode, bool shiftDown = false, bool CTRLDown = false, bool altDown = false);
        [[nodiscard]] bool isKeyPressed() const;
        [[nodiscard]] bool isKeyDown() const;
        // Returns only the release of the base key
        [[nodiscard]] bool isKeyReleased() const;

        // Returns the base key code
        [[nodiscard]] int getKey() const;

        [[nodiscard]] bool hasShift() const;
        [[nodiscard]] bool hasCtrl() const;
        [[nodiscard]] bool hasAlt() const;

    private:
        uint16_t data = 0;
    };

    // Efficient representation of a setting with mulitple different types
    struct Setting final
    {
        // Supported types:
        Setting() = default;
        explicit Setting(Vector2& val);
        explicit Setting(int val);
        explicit Setting(bool val);
        explicit Setting(float val);

        // Retrieve the settings values
        // IMPORTANT: needs to be the same type that was used to save!
        template <typename T>
        T get() const;

        // Saves this value for the setting
        // Can be used to override datatype aswell
        template <typename T>
        void save(const T& value);

    private:
        int64_t data = 0;
    };


    //----------------- MULTIPLAYER -----------------//

    enum UpdateFlag : uint8_t
    {
        UPDATE_DELETE_ENTITY = 1,
        UPDATE_POSITION_ENTITY = 2,
        UPDATE_HEALTH_ENTITY = 4,
        UPDATE_SPAWN_ENTITY = 8,
        FILLER2 = 16,
        FILLER3 = 32,
        FILLER4 = 64,
        FILLER5 = 128,
    };


    enum UDP_Channel : uint8_t
    {
        //-----------FOR-HOST-----------//
        HOST_PLAYER_ACTION,
        HOST_CHARACTER_INFO,

        //-----------FOR-CLIENT-----------//
        CLIENT_PLAYER_NAME_UPDATE,
        CLIENT_ID_ASSIGN,
        CLIENT_ENTITY_POS,
        CLIENT_ENTITY_POS_STAT,
        CLIENT_ENTITY_STAT,
        CLIENT_ENTITY_SPAWN,
        CLIENT_ENTITY_DESPAWN,
        CLIENT_ABILITY_USED,
        CLIENT_QUEST_UPDATE,
        CLIENT_EFFECT_UPDATE,
    };


} // namespace magique


#endif //MAGIQUE_TYPES_H