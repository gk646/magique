#ifndef MAGIQUE_TYPES_H
#define MAGIQUE_TYPES_H

#include "magique/fwd.hpp"

//-----------------------------------------------
// Types Modules
//-----------------------------------------------
// ................................................................................
// These are the simple and public types/enums used and exposed by magique
// Note that some enum use a explicit type to save memory when used in the ECS or networking
// ................................................................................

namespace magique
{
    //----------------- ASSETS  -----------------//

    struct TextureRegion final // All textures are part of an atlas and can not be referenced as standalone
    {
        uint16_t offX;  // Horizontal offset from the top left of the atlas
        uint16_t offY;  // Vertical offset from the top left of the atlas
        int16_t width;  // Width of the texture
        int16_t height; // Height of the texture
        uint16_t id;    // The texture id
    };

    struct SpriteSheet final
    {
        uint16_t offX;   // Horizontal offset from the top left of the atlas of the first frame
        uint16_t offY;   // Vertical offset from the top left of the atlas of the first frame
        int16_t width;   // Width of a frame
        int16_t height;  // Height of a frame
        uint16_t id;     // The texture id
        uint16_t frames; // Total number of frames
    };

    struct Animation final
    {
        SpriteSheet sheet{};
        uint16_t duration = UINT16_MAX;

        [[nodiscard]] int getCurrentTexture(uint16_t spriteCount) const;
    };

    //----------------- ENTITY COMPONENT SYSTEM -----------------//

    // Default action states
    enum class ActionState : uint8_t
    {
        IDLE,
        WALK,
        RUN,
        ATTACK_1,
        ATTACK_2,
        HIT,
        SPECIAL,
        DEATH,
        STATES_END, // All custom state enums need this as last state
    };

    // Which lighting style the emitter has
    enum LightStyle : uint8_t
    {
        POINT_LIGHT_SOFT,         // Point ligtht
        DIRECTIONAL_LIGHT_STRONG, // Sunlight
    };

    // Shape classes
    enum Shape : uint8_t
    {
        CIRCLE, // Circle
        RECT,   // Rectangle, can be rotated
        POLYGON,
    };

    // Feel free to rename those!
    enum class CollisionLayer : uint8_t
    {
        DEFAULT_LAYER = 1 << 0,
        LAYER_1 = 1 << 1,
        LAYER_2 = 1 << 2,
        LAYER_3 = 1 << 3,
        LAYER_4 = 1 << 4,
        LAYER_5 = 1 << 5,
        LAYER_6 = 1 << 6,
        LAYER_7 = 1 << 7,
    };

    //----------------- UI -----------------//

    // Anchor position used in the UI module to position objects
    enum class AnchorPosition
    {
        LEFT_TOP,      // LT
        LEFT_MID,      // LM
        LEFT_BOTTOM,   // LB
        CENTER_TOP,    // CT
        CENTER_MID,    // CM
        CENTER_BOTTOM, // CB
        RIGHT_TOP,     // RT
        RIGHT_MID,     // RM
        RIGHT_BOTTOM   // RB
    };

    enum Size
    {
        MINI,
        SMALL,
        MID,
        BIG
    };

    //----------------- HELPER TYPES -----------------//

    // Efficient representation of a keybind with optional modifiers
    struct Keybind final
    {
        Keybind() = default;
        explicit Keybind(int keyCode, bool shiftDown = false, bool CTRLDown = false, bool altDown = false);

        // Returns true if the keybind is pressed
        [[nodiscard]] bool isPressed() const;

        // Returns true if the keybind is down
        [[nodiscard]] bool isDown() const;

        // Returns true if the base key OR any modifiers are released
        [[nodiscard]] bool isReleased() const;

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
        Setting() = default;
        // Supported types:
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

    // Efficient representation of a achievement - done when the condition is true once
    struct Achievement final
    {
        bool finished = false;
        const char* name = nullptr;
        void* condition;

        Achievement(const char* name, void* condition) : name(name), condition(condition) {}
        ~Achievement();
    };

    //----------------- MISC -----------------//

    struct Point final
    {
        float x;
        float y;
    };

    struct CursorAttachment final
    {
        void* userPointer;
    };

    enum class LightingModel : uint8_t
    {
        STATIC_SHADOWS, // Default
        RAY_TRACING,
        NONE,
    };

    enum class KeyLayout
    {
        QWERTY,
        QWERTZ,
        AUTOMATIC,
    };

    // Used in any of the loader interfaces
    // Priority is handled based on semantic meaning e.g. MEDIUM is before LOW
    enum PriorityLevel
    {
        LOW,
        MEDIUM,
        HIGH,
        CRITICAL,
        INSTANT,
    };

    enum Thread
    {
        MAIN_THREAD,
        BACKGROUND_THREAD,
    };

    enum TextAlign
    {
        LEFT,
        CENTERED,
        RIGHT
    };

    //----------------- MULTIPLAYER -----------------//

    enum class SendFlag : uint8_t
    {
        // Reliable message send. Can send up to k_cbMaxSteamNetworkingSocketsMessageSizeSend bytes in a single message.
        // Does fragmentation/re-assembly of messages under the hood, as well as a sliding window for
        // efficient sends of large chunks of data.
        //
        // The Nagle algorithm is used.
        RELIABLE = 8,
        // Send the message unreliably. Can be lost.  Messages *can* be larger than a
        // single MTU (UDP packet), but there is no retransmission, so if any piece
        // of the message is lost, the entire message will be dropped.
        //
        // The sending API does have some knowledge of the underlying connection, so
        // if there is no NAT-traversal accomplished or there is a recognized adjustment
        // happening on the connection, the packet will be batched until the connection
        // is open again.
        UN_RELIABLE = 0,
    };

    enum class Connection : uint32_t
    {
        INVALID_CONNECTION = 0,
    };

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