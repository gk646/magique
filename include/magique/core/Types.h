#ifndef TYPES_H
#define TYPES_H

#include "magique/fwd.hpp"
#include "entt/entity/fwd.hpp"

//-----------------------------------------------
// Types Modules
//-----------------------------------------------

// .....................................................................
// These are common types used by the engine
// .....................................................................

namespace magique
{
    enum MapID : uint8_t; // Models different levels

    struct Point final
    {
        float x;
        float y;
    };

    struct Pint final
    {
        int x;
        int y;
    };


    struct TextureRegion final // All texture are part of an atlas and can not be referenced as standalone
    {
        uint16_t offX;   // Horizontal offset from the top left of the atlas
        uint16_t offY;   // Vertical offset from the top left of the atlas
        uint16_t width;  // Width of the texture
        uint16_t height; // Height of the texture
        uint16_t id;     // The texture id
    };


    struct SpriteSheet final
    {
        uint16_t offX;   // Horizontal offset from the top left of the atlas of the first frame
        uint16_t offY;   // Vertical offset from the top left of the atlas of the first frame
        uint16_t width;  // Width of a frame
        uint16_t height; // Height of a frame
        uint16_t id;     // The texture id
        uint16_t frames; // Total number of frames
    };

    // The information used through a single draw tick
    // The information is guaranteed to not change during a tick!
    struct DrawTickData final
    {
        const Map* currentMap = nullptr;
        Pint cameraTilePosition{};
        Point cameraPosition{};
        entt::entity camera{};
        MapID currentZone{};
        float visualScaleX = 1.0F;
        float visualScaleY = 1.0F;
    };


    //----------------- MULTIPLAYER -----------------//

    enum UpdateFlag : uint8_t
    {
        //-----------UPDATE_TYPES-----------//
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


#endif //TYPES_H