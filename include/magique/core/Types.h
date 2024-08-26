#ifndef MAGIQUE_TYPES_H
#define MAGIQUE_TYPES_H

#include <magique/fwd.hpp>

//-----------------------------------------------
// Types Modules
//-----------------------------------------------
// ................................................................................
// These are the simple and public types/enums used and exposed by magique
// Note: Some enum use a explicit type to save memory when used in the ECS or networking
// ................................................................................

namespace magique
{
    struct Point final
    {
        float x;
        float y;
    };

    //----------------- CORE -----------------//

    enum class LightingMode
    {
        STATIC_SHADOWS,
        RAY_TRACING,
        NONE, // Default
    };

    //----------------- ASSETS  -----------------//

    enum AtlasID : uint8_t // Can add new ones or rename them
    {
        DEFAULT, // Default atlas
        CHARACTER,
        USER_INTERFACE, // All UI related textures
        ENTITIES_1,
        ENTITIES_2,
        ENTITIES_3,
        ATLAS_END // Always needs to be last
    };

    // Used in any of the loader interfaces
    // Priority is handled based on semantic meaning e.g. MEDIUM is before LOW
    enum PriorityLevel
    {
        LOW,
        MEDIUM,
        HIGH,
        CRITICAL,
        INTERNAL, // Reserved
    };

    enum ThreadType
    {
        MAIN_THREAD,
        BACKGROUND_THREAD,
    };

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

    // Objects saved with the tilemap
    struct TileObject final
    {
        [[nodiscard]] const char* getName() const; // Can be null
        [[nodiscard]] int getClass() const;        // Only ints allows as class
        [[nodiscard]] int getID() const;
        [[nodiscard]] Rectangle getRect() const;

        float x = 0, y = 0, width = 0, height = 0; // Mutable
        bool visible = false;                      // Mutable

    private:
        char* name = nullptr;
        int type = -1; // Class
        int id = -1;
        friend TileObject ParseObject(const char*);
    };

    struct TileInfo final
    {
        uint16_t tileID = UINT16_MAX; // ID of the tile
        int clazz = 0;                // class attribute
        float probability = 1.0F;     // probability attribute
    };

    //----------------- ECS -----------------//

    // Default action states - Feel free to rename or create your own
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
    enum class Shape : uint8_t
    {
        RECT,     // Rectangle
        CIRCLE,   // Circle - rotated around its middle point
        CAPSULE,  // Capsule (vertical - non rotated) - https://docs.unity3d.com/Manual/class-CapsuleCollider2D.html
        TRIANGLE, // Triangle
    };

    // Feel free to rename those!
    enum CollisionLayer : uint8_t
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

    enum class ColliderType : uint8_t
    {
        WORLD_BOUNDS,
        TILEMAP_OBJECT,
        SOLID_TILE,
        MANUAL_COLLIDER,
    };

    struct ColliderInfo final
    {
        uint16_t data;
        ColliderType type;
    };

    struct StaticCollider final
    {
        float x, y;   // Position
        float p1, p2; // Extra values - if p2 == 0 -> circle
    };

    struct CollisionInfo final
    {
        Point normalVector{};   // The direction  vector in which the object needs to be mover to resolve the collision
        Point collisionPoint{}; // The point of contact (or often the closest point on the shapes between the centers)
        float penDepth = 0;     // The amount by which the shapes overlap - minimal distance to move along the normal

        // Returns true
        [[nodiscard]] bool isColliding() const;
    };

    //----------------- GAMEDEV -----------------//

    enum class NoiseType
    {
        OPEN_SIMPLEX_2,
        OPEN_SIMPLEX_2S,
        CELLULAR,
        PERLIN,
        VALUE_CUBIC,
        VALUE,
    };

    //----------------- MULTIPLAYER -----------------//

    enum class SendFlag : uint8_t
    {
        // Reliable message send. Does fragmentation/re-assembly of messages under the hood, as well as a sliding window
        // for efficient sends of large chunks of data - The Nagle algorithm is used.
        RELIABLE = 8,
        // Send the message unreliably. Can be lost.  Messages *can* be larger than a
        // single MTU (UDP packet), but there is no retransmission, so if any piece
        // of the message is lost, the entire message will be dropped.
        UN_RELIABLE = 0,
    };

    enum class Connection : uint32_t
    {
        INVALID_CONNECTION = 0,
    };

    enum class MultiplayerEvent : uint8_t
    {
        //----------------- HOST -----------------//
        HOST_NEW_CONNECTION,           // Posted when when we accept a new client connection
        HOST_CLIENT_CLOSED_CONNECTION, // Posted when when the client closed the connection
        //----------------- CLIENT -----------------//
        CLIENT_CONNECTION_ACCEPTED, // Posted when the host accepted our connection
        CLIENT_CONNECTION_CLOSED,   // Posted when the host closed our connection
    };

    struct Payload final
    {
        const void* data; // Direct pointer to the given data
        int size;         // Valid size of the data
        MessageType type; // Type of the message (very useful for handling messages on the receiver)
    };

    struct Message final
    {
        Payload payload;       // Same payload that was sent
        Connection connection; // Who sent the payload
        int64_t timeStamp;     // When the message was received (micros) - should only be compared to other timestamps
    };

    //----------------- PERSISTENCE -----------------//

    enum class StorageType : uint8_t // The type of the storage cell
    {
        STRING,     // Stores a string
        DATA,       // Stores arbitrary data
        VECTOR,     // Stores (typed) vector data
        DATA_TABLE, // Saves data for a magique::DataTable
        KEY_BIND,   // Stores a key bind (used only by GameConfig)
        VALUE,      // Stores a 8 bytes value (used only by GameConfig)
        EMPTY,      // Storage is empty
    };

    //----------------- STEAM -----------------//

    enum class SteamID : uint64_t;

    //----------------- UI -----------------//

    // Anchor position used in the UI module to position objects
    enum class AnchorPosition
    {
        TOP_LEFT,      // LT
        MID_LEFT,      // LM
        BOTTOM_LEFT,   // LB
        TOP_CENTER,    // CT
        MID_CENTER,    // CM
        BOTTOM_CENTER, // CB
        TOP_RIGHT,     // RT
        MID_RIGHT,     // RM
        BOTTOM_RIGHT   // RB
    };

    enum class KeyLayout
    {
        QWERTY,
        QWERTZ,
        AUTOMATIC,
    };

    enum Size
    {
        MINI,
        SMALL,
        MID,
        BIG
    };

    // The render order of ui elements - the higher the layer the closer the element is to the screen
    enum class UILayer : uint8_t
    {
        BACK_GROUND,
        LOW,
        MEDIUM,
        HIGH,
        ONTOP,
        ROOT,
    };

    struct KeyEvent final
    {
        bool isShift() const { return shift; }

        int getKey() const { return key; }

    private:
        int key = 0;
        bool shift = false;
        bool alt = false;
        bool control = false;
    };

    struct MouseEvent final
    {
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

    struct ScreenParticle final
    {
        float x, y;                   // Position
        int16_t p1;                   // RECT: width  / CIRCLE: radius  / CAPSULE: radius  / TRIANGLE: offsetX
        int16_t p2;                   // RECT: height                   / CAPSULE: heigth  / TRIANGLE: offsetY
        int16_t p3;                   // RECT: height                   / CAPSULE: heigth  / TRIANGLE: offsetY
        int16_t p4;                   // RECT: height                   / CAPSULE: heigth  / TRIANGLE: offsetY
        float vx, vy;                 // Velocity
        float scale;                  // Current scale
        uint16_t age;                 // Current age
        uint16_t lifeTime;            // Lifetime
        Shape shape;                  // Shape
        uint8_t r, g, b, a;           // Current color
        const ScreenEmitter* emitter; // Function pointers are shared across all instances

        [[nodiscard]] Color getColor() const;
        void setColor(const Color& color);
    };

    // Loading task - has to be subclassed with the correct type for T (e.g AssetContainer or GameSave...)
    template <typename T>
    struct ITask
    {
        virtual ~ITask() = default;
        virtual void execute(T& res) = 0;

        [[nodiscard]] bool getIsLoaded() const { return isLoaded; }
        [[nodiscard]] int getImpact() const { return impact; }

    private:
        bool isLoaded = false;
        int impact = 0;
        friend struct internal::TaskExecutor<T>;
    };

    //----------------- MISC -----------------//

    // Pointer will always be allocated with new []
    template <typename T>
    struct DataPointer final
    {
        DataPointer(T* pointer, const int size) : pointer(pointer), size(size) {}
        ~DataPointer() noexcept { free(); }

        // Returns size in bytes
        [[nodiscard]] int getSize() const { return size; }

        // Returns the underlying data pointer
        T* getData() const { return pointer; }

        void free() noexcept
        {
            delete[] pointer;
            pointer = nullptr;
        }

    private:
        T* pointer; // The data pointer
        int size;   // The size of the data pointer in bytes
        friend void UnCompressImage(char*&, int&);
    };

    // Array Iterator template
    template <typename U>
    class Iterator
    {
    public:
        using value_type = U;
        using difference_type = int64_t;
        using pointer = U*;
        using reference = U&;

        explicit Iterator(pointer ptr) : ptr_(ptr) {}

        reference operator*() const { return *ptr_; }

        pointer operator->() { return ptr_; }

        Iterator& operator++()
        {
            ++ptr_;
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++ptr_;
            return tmp;
        }

        Iterator& operator--()
        {
            --ptr_;
            return *this;
        }

        Iterator operator--(int)
        {
            Iterator tmp = *this;
            --ptr_;
            return tmp;
        }

        Iterator& operator+=(difference_type offset)
        {
            ptr_ += offset;
            return *this;
        }

        Iterator operator+(difference_type offset) const { return Iterator(ptr_ + offset); }

        Iterator& operator-=(difference_type offset)
        {
            ptr_ -= offset;
            return *this;
        }

        Iterator operator-(difference_type offset) const { return Iterator(ptr_ - offset); }

        difference_type operator-(const Iterator& other) const { return ptr_ - other.ptr_; }

        reference operator[](difference_type index) const { return ptr_[index]; }

        bool operator==(const Iterator& other) const { return ptr_ == other.ptr_; }

        bool operator!=(const Iterator& other) const { return ptr_ != other.ptr_; }

        bool operator<(const Iterator& other) const { return ptr_ < other.ptr_; }

        bool operator>(const Iterator& other) const { return ptr_ > other.ptr_; }

        bool operator<=(const Iterator& other) const { return ptr_ <= other.ptr_; }

        bool operator>=(const Iterator& other) const { return ptr_ >= other.ptr_; }

    private:
        pointer ptr_;
    };

} // namespace magique


#endif //MAGIQUE_TYPES_H