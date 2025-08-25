// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_TYPES_H
#define MAGIQUE_TYPES_H

#include <magique/fwd.hpp>

//===============================================
// Types Modules
//===============================================
// ................................................................................
// These are the simple and public types/enums used and exposed by magique
// Note: Some enum use an explicit type to save memory when used in the ECS or networking
// ................................................................................

namespace magique
{
    struct Point final
    {
        float x;
        float y;

        Point() = default;
        Point(const Vector2& vec);
        constexpr Point(float x, float y) : x(x), y(y) {}

        // With other points
        bool operator==(const Point& p) const;
        bool operator!=(const Point& p) const;
        Point operator+(const Point& p) const;
        Point operator-(const Point& point) const;
        Point& operator+=(const Point& p);
        Point& operator-=(const Point& p);
        Point operator*(const Point& other) const;
        Point& operator*=(const Point& p);
        Point operator/(const Point& p) const;

        // With numbers
        [[nodiscard]] Point operator*(float i) const;
        bool operator<(float num) const;  // Both must be smaller
        bool operator==(float num) const; // Both must be equal
        Point operator/(float divisor) const;
        Point operator-(float f) const;
        Point operator+(float f) const; // To both
        Point& operator*=(float f);
        Point& operator=(float f); // Assign to both
        Point& operator-=(float f);
        Point& operator/=(float f);

        // Distance functions
        [[nodiscard]] float manhattan(const Point& p) const;
        [[nodiscard]] float euclidean(const Point& p) const;
        [[nodiscard]] float chebyshev(const Point& p) const;
        [[nodiscard]] float octile(const Point& p) const;

        [[nodiscard]] Vector2 v() const; // Convert into raylib vector

        // Vector functions

        // How much vectors point in the same direction - positive if less than 90 degrees - negative if more
        float dot(const Point& p) const;

        // Returns the relative orientation of two vectors (and the area of the parallelogram they span)
        // Positive if second vector is to the left, negative if to the right (from the first)
        float cross(const Point& p) const;

        // Inverts the vectors direction
        Point& invert();

        // Vector normalization - with Euclidean method (L2) max length is 1.4 (creates circle shape)
        Point& normalize();

        // Vector normalization with manhattan method (L1) max length is 1 (creates diamond shape)
        Point& normalizeManhattan();

        float magnitude() const;

        // uses std::round() to round to the nearest whole number
        Point& round();

        // uses std::floor() to round to the closest whole to the left (watch out for negative numbers)
        Point& floor();

        // Clamps both values inside the given range - if outside the range will be set to the closes point in range
        Point& clamp(float min, float max);

        // Decreases the magnitude of the vector by the given scalar (flat)
        Point& decreaseMagnitude(float f);

        // Returns the perpendicular vector to this one - either to the left or to the right
        Point perpendicular(bool left) const;

        // Given two points in world space returns a direction vector that is perpendicular to the given direction
        // Useful when you want to knock things out of your way
        static Point PerpendicularTowardsPoint(const Point& startPoint, const Point& direction, const Point& target);
    };

    //================= CORE =================//

    enum class LightingMode
    {
        NONE,           // Default
        STATIC_SHADOWS, // Fast but only hard shadows - max of 1k+ objects
        RAY_TRACING,    // Slow but provides global illumination - max of ~50 object
    };

    //================= ASSETS =================//

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
        THREAD_MAIN,
        THREAD_ANY,
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

        [[nodiscard]] TextureRegion getRegion(int frame) const;
    };

    using DurationArray = uint8_t[MAGIQUE_MAX_ANIM_FRAMES]; // Duration in ticks

    struct SpriteAnimation final
    {
        DurationArray durations{};
        SpriteSheet sheet{};
        uint16_t maxDuration = 0;
        int16_t offX = 0; // Draw offset
        int16_t offY = 0;
        int16_t rotX = 0; // Rotation anchor
        int16_t rotY = 0;

        [[nodiscard]] TextureRegion getCurrentFrame(uint16_t spriteCount) const;

        [[nodiscard]] Point getAnchor() const;
    };

    // The type of the property
    enum class TileObjectPropertyType : uint8_t
    {
        NONE,
        INT,
        BOOL,
        FLOAT,
        STRING,
        FILE, // just a string
        COLOR,
    };

    // A custom definable property inside the Tiled Tile Editor
    // -> go to your TileSet -> click on a tile -> right click on custom properties -> choose type, name and value
    struct TileObjectCustomProperty final
    {
        // Returns the value
        // IMPORTANT: program will crash when you call the wrong type getter
        //            -> e.g. check if it's an integer first before calling getInt()
        [[nodiscard]] bool getBool() const;
        [[nodiscard]] int getInt() const;
        [[nodiscard]] float getFloat() const;
        [[nodiscard]] const char* getString() const;
        [[nodiscard]] Color getColor() const;

        [[nodiscard]] TileObjectPropertyType getType() const;

        [[nodiscard]] const char* getName() const;

    private:
        char* name = nullptr;
        TileObjectPropertyType type = TileObjectPropertyType::INT;
        union
        {
            float floating;
            int integer{};
            char* string;
            bool boolean;
        };
        friend TileMap ImportTileMap(Asset asset);
    };

    // Objects defined inside the tile editor
    struct TileObject final
    {
        const char* getName() const; // Can be null
        int getID() const;
        int getClass() const;

        float x = 0, y = 0, width = 0, height = 0;                                        // Mutable
        bool visible = false;                                                             // Mutable
        TileObjectCustomProperty customProperties[MAGIQUE_TILE_OBJECT_CUSTOM_PROPERTIES]; // Mutable

    private:
        int class_ = 0;
        char* name = nullptr;
        int id = INT32_MAX;
        friend TileMap ImportTileMap(Asset asset);
    };

    struct TileInfo final
    {
        uint8_t x = 0, y = 0, width = 0, height = 0;
        uint16_t tileID = UINT16_MAX; // ID of the tile

        [[nodiscard]] int getClass() const;

    private:
        int tileClass = INT32_MAX; // class attribute
        friend struct TileSet;
    };

    // Checksum (hash) for a file
    struct Checksum final
    {
        // Initializes the checksum - should be the output of print() or format() or another MD5 implementation
        explicit Checksum(const char* hexadecimalHash);

        bool operator==(const Checksum& other) const;

        // Prints the checksum in hexadecimal digits to stdout
        void print() const;

        // Formats the checksum into the given buffer
        void format(char* buffer, int size) const;

    private:
        Checksum() = default;
        uint32_t first = 0;
        uint32_t second = 0;
        uint32_t third = 0;
        uint32_t fourth = 0;
        friend Checksum GetAssetImageChecksum(const char* path);
        friend bool ValidateAssetImage(Checksum, const char*);
    };

    //================= ECS =================//

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
        POINT_LIGHT_SOFT,         // Point light
        DIRECTIONAL_LIGHT_STRONG, // Sunlight
    };

    // Shape classes
    enum class Shape : uint8_t
    {
        RECT,     // Rectangle
        CIRCLE,   // Circle - rotated around its middle point
        CAPSULE,  // Capsule (vertical - non rotated) - https://docs.unity3d.com/Manual/class-CapsuleCollider2D.html
        TRIANGLE, // Triangle - should only be used as detection shape not colliders (normals are bit funky)
    };

    // Feel free to rename those! This is a bit mask!
    enum class CollisionLayer : uint8_t
    {
        NONE = 0,
        DEFAULT_LAYER = 1 << 0,
        LAYER_1 = 1 << 1,
        LAYER_2 = 1 << 2,
        LAYER_3 = 1 << 3,
        LAYER_4 = 1 << 4,
        LAYER_5 = 1 << 5,
        LAYER_6 = 1 << 6,
    };

    enum class ColliderType : uint8_t
    {
        WORLD_BOUNDS,
        TILEMAP_OBJECT,
        TILESET_TILE,
        MANUAL_COLLIDER,
    };

    struct ColliderInfo final
    {
        // Note: If you used the wrong getter (for the type) returns INT32_MAX with a warning

        // Returns the collider class ONLY IF the type is TILEMAP_OBJECT
        [[nodiscard]] int getColliderClass() const;

        // Returns the group number ONLY IF the type is MANUAL_COLLIDER
        [[nodiscard]] int getManualGroup() const;

        // Returns the tile class ONLY IF the type is
        [[nodiscard]] int getTileNum() const;

        const ColliderType type; // The type of the collider

        ColliderInfo(int data, ColliderType type);

    private:
        int data;
    };

    struct StaticCollider final
    {
        float x, y;   // Position
        float p1, p2; // Extra values - if p2 == 0 -> circle
    };

    struct CollisionInfo final
    {
        Point normalVector{};   // The direction vector in which the object needs to be moved to resolve the collision
        Point collisionPoint{}; // The point of contact (or often the closest point on the shapes between the centers)
        float penDepth = 0;     // The amount by which the shapes overlap - minimal distance to move along the normal

        // Returns true
        [[nodiscard]] bool isColliding() const;

        [[nodiscard]] bool getIsAccumulated() const;

    private:
        bool isAccumulated = false; // True if this info should be accumulated for this entity
        friend void SetIsAccumulated(CollisionInfo& info);
    };

    // Called BEFORE the entity is destroyed with its info
    using DestroyEntityCallback = void (*)(entt::entity entity, const PositionC& position);

    //================= GAMEDEV =================//

    enum class NoiseType
    {
        OPEN_SIMPLEX_2,
        OPEN_SIMPLEX_2S,
        CELLULAR,
        PERLIN,
        VALUE_CUBIC,
        VALUE,
    };

    enum class ParamType
    {
        // [Boolean type]
        // Can either be 0 or 1
        // Parsing rules:
        //      - 0: False false FALSE OFF off
        //      - 1: True true TRUE ON on
        BOOL = 1,
        // [Number type]
        // Parsing rules:
        // All numeric characters [0,1,2,3,4,5,6,7,8,9] optionally separated only by a single "."
        //      - 123.333 -> single valid number
        //      - 123 333 -> two valid numbers
        //      - 3a3     -> parsed as string
        NUMBER,
        // [String type]
        // Only ASCII characters supported
        // Parsing rules:
        //      - Everything that is not a NUMBER or BOOL is a string
        STRING,
        // Examples:
        //      - 123 ON false  -> NUMBER BOOL BOOL
        //      - 123 abc 1.4   -> NUMBER STRING NUMBER
        //      - 123a abc 1.4  -> STRING STRING NUMBER
    };

    struct Param final
    {
        Param(const char* name, float value);
        Param(const char* name, const char* value);
        Param(const char* name, bool value);

        // Returns the parameter name
        const char* getName() const;

        // Returns the parameter string values
        // Note: MUST not be stored by value - copy it instead if you want to save it
        const char* getString() const;

        // Returns the parameters boolean value
        bool getBool() const; // Returns the parameters boolean value

        // Returns the parameters float value
        float getFloat() const;

        // Returns the parameters integer value
        int getInt() const;

        // Returns the type of the parameter
        ParamType getType() const;

        Param() = default;

    private:
        union
        {
            float number;
            bool boolean;
            const char* string;
        };
        const char* name;
        ParamType type; // Type of the parameter
        friend ParamParser;
        friend void SetEnvironmentParam(const char*, const char*);
        friend void SetEnvironmentParam(const char*, float);
        friend void SetEnvironmentParam(const char*, bool);
        friend bool RemoveEnvironmentParam(const char*);
    };

    enum class GridMode : uint8_t
    {
        // Only allows movement in four orthogonal directions, up left down right
        // The found path will NOT contain any diagonal moves
        CROSS,
        STAR, // Allows all orthogonal direction and additionally all diagonals top left, top right...
    };

    //================= MULTIPLAYER =================//

    enum class SendFlag : uint8_t
    {
        // No guarantees - the message is not resent if its dropped or part of it goes missing
        // This uses less bandwidth than reliable messages - if there's a stable connection it's very similar to reliable
        UNRELIABLE = 0,
        // There are TWO guarantees when using reliable message:
        //      1. They are guaranteed to arrive (if its possible) -> a confirmation is sent back from the client if it was received
        //      2. Reliable messages retain order, they arrive at the client in the same order they were sent from the host
        // Use this for vital game updates and messages that MUST arrive or arrive in a certain order
        RELIABLE = 8,


    };

    enum class Connection : uint32_t
    {
        INVALID_CONNECTION = 0,
    };

    enum class MultiplayerEvent : uint8_t
    {
        //================= HOST =================//
        // Posted after we accept a new client connection
        HOST_NEW_CONNECTION,
        // Posted after the client closed the connection
        HOST_CLIENT_DISCONNECTED,
        // Posted if a local problem terminated a client connection (usually because client leaves not gracefully)
        HOST_LOCAL_PROBLEM,

        //================= CLIENT =================//
        // Posted after the host accepted our connection
        CLIENT_CONNECTION_ACCEPTED,
        // Posted after the host closed our connection
        CLIENT_CONNECTION_CLOSED,
        // Posted if a local problem terminated the connection to the host (usually because host leaves not gracefully)
        CLIENT_LOCAL_PROBLEM,
    };

    struct Payload final
    {
        const void* data; // Direct pointer to the given data
        int size;         // Valid size of the data
        MessageType type; // Type of the message (very useful for handling messages on the receiver)

        // Cast the payload data to an object of the given type and returns it - uses *static_cast
        template <typename T>
        T getDataAs() const;
    };

    struct Message final
    {
        Payload payload;       // Same payload that was sent
        Connection connection; // Who sent the payload
        int64_t timeStamp;     // When the message was received (micros) - should only be compared to other timestamps
    };

    //================= PERSISTENCE =================//

    enum class StorageType : uint8_t // The type of the storage cell
    {
        STRING,     // Stores a string
        DATA,       // Stores arbitrary data
        VECTOR,     // Stores (typed) vector data
        DATA_TABLE, // Saves data for a magique::DataTable
        KEY_BIND,   // Stores a key bind (used only by GameConfig)
        VALUE,      // Stores an 8 bytes value (used only by GameConfig)
        EMPTY,      // Storage is empty
    };

    //================= STEAM =================//

    enum class SteamLobbyID : uint64_t; // Steam lobby ID - internally is just a SteamID

    enum class SteamID : uint64_t; // SteamID

    using SteamOverlayCallback = void (*)(bool isOpening);

    enum class SteamLobbyType
    {
        PRIVATE,
        FRIENDS_ONLY,
        PUBLIC,
    };

    enum class SteamLobbyEvent
    {
        // Called only when you created a lobby - called with your id
        ON_LOBBY_CREATED,
        // Called when you enter a lobby - called with your id
        ON_LOBBY_ENTERED,
        // Called when you exit the lobby - called with your id
        ON_LOBBY_EXIT,
        // Only called when you are in a lobby and someone ELSE joins the lobby - called with joiners id
        ON_USER_JOINED,
        // Only called when you are in a lobby and someone ELSE leaves the lobby - called with leaver id
        ON_USER_LEFT,

        // Called when a friend sends you a lobby invite - called with others lobby id, called with inviters id
        // Note: It's up to you to handle a few things:
        //      - Handle the case you are in a lobby currently
        //          - Leave the current lobby or display a confirmation message
        //      - Join the new lobby with JoinSteamLobby(lobbyID);
        ON_LOBBY_INVITE,
    };

    //================= UI =================//

    // Anchor position used in the UI module to position objects
    // Note: Look at ui/UI.h for more info on positioning of anchor points
    enum class Anchor : uint8_t
    {
        NONE,          // No anchoring - for UIObject
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

    // When used for alignment positions the object in the specified direction
    enum class Direction : uint8_t
    {
        LEFT,  // Object is moved left from the anchor object by its width
        RIGHT, // Object is moved right from where the anchor object ends horizontally by its width
        UP,    // Object is moved up from the anchor object by its height
        DOWN   // Object is moved up down from where the anchor object ends vertically by its height
    };

    enum class KeyLayout : uint8_t
    {
        QWERTY,
        QWERTZ,
    };

    enum class ScalingMode : uint8_t
    {
        // Fully scales with the current screen dimensions - used for background, or windows
        // Note: if the current aspect ratio is not 16:9, the object will be stretched or squished accordingly
        FULL,
        // Scales only the dimensions by the current screen height - used for most things and with a distinct shape (square...)
        // In this mode you should use anchor points or align() to position the object as the static position will not fit the size anymore
        KEEP_RATIO,
        // Object is not changed at all by different resolutions - should only be used in special cases
        NONE,
    };

    enum class SliderMode : uint8_t
    {
        BALANCED,
        IMBALANCED,
    };

    using ClickFunc = void (*)(const struct Rectangle& bounds, int mouseButton);

    //================= HELPER TYPES =================//

    // Efficient representation of a keybind with optional modifiers
    // Also works with mouse buttons e.g. enum MouseButton (left, right and middle)
    struct Keybind final
    {
        Keybind() = default;
        explicit Keybind(int key, bool isLayered = true, bool shift = false, bool ctrl = false, bool alt = false);

        // Uses direct input polling (e.g. IsKeyDown())
        [[nodiscard]] bool isPressed() const;
        [[nodiscard]] bool isDown() const;
        // Returns true if the base key OR any modifiers are released
        [[nodiscard]] bool isReleased() const;

        // Returns the base key code
        [[nodiscard]] int getKey() const;
        [[nodiscard]] bool hasShift() const;
        [[nodiscard]] bool hasCtrl() const;
        [[nodiscard]] bool hasAlt() const;
        [[nodiscard]] bool isUIInput() const;

        // Returns true if its keybind for a mouse button
        bool isMouse() const;

    private:
        uint16_t key = 0;
        bool shift = false;
        bool ctrl = false;
        bool alt = false;
        bool layered = false;
    };

    struct ScreenParticle final
    {
        Point pos;
        int16_t p1;                   // RECT: width  / CIRCLE: radius
        int16_t p2;                   // RECT: height
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

    // Loading task - has to be subclassed with the correct type for T (e.g. AssetContainer or GameSaveData...)
    template <typename T>
    struct ITask
    {
        virtual ~ITask() = default;

        // Main execution method - passed a modifiable reference to the used resource/container
        virtual void execute(T& res) = 0;

        // Returns true if the task has been loaded
        [[nodiscard]] bool getIsLoaded() const;

        // Returns the subjective arbitrary impact measurement - set by the user
        [[nodiscard]] int getImpact() const;

    private:
        bool isLoaded = false;
        int impact = 0;
        friend struct internal::TaskExecutor<T>;
    };

    //================= MISC =================//

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
    template <typename T>
    class Iterator
    {
    public:
        using value_type = T;
        using difference_type = int64_t;
        using pointer = T*;
        explicit Iterator(pointer ptr) : ptr_(ptr) {}
        T& operator*() { return *ptr_; }
        const T& operator*() const { return *ptr_; }
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
        Iterator operator+(difference_type offset) const { return Iterator(ptr_ + offset); }
        Iterator operator-(difference_type offset) const { return Iterator(ptr_ - offset); }
        difference_type operator-(const Iterator& other) const { return ptr_ - other.ptr_; }
        T& operator[](difference_type index) const { return ptr_[index]; }
        bool operator==(const Iterator& other) const { return ptr_ == other.ptr_; }
        bool operator!=(const Iterator& other) const { return ptr_ != other.ptr_; }

    private:
        pointer ptr_;
    };


} // namespace magique

//================= IMPLEMENTATION =================//

namespace magique
{
    template <typename T>
    T Payload::getDataAs() const
    {
        return *static_cast<const T*>(data);
    }

    template <typename T>
    bool ITask<T>::getIsLoaded() const
    {
        return isLoaded;
    }

    template <typename T>
    int ITask<T>::getImpact() const
    {
        return impact;
    }
} // namespace magique


#endif //MAGIQUE_TYPES_H