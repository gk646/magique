#ifndef STATICCOLLISIONDATA_H
#define STATICCOLLISIONDATA_H

#include <magique/util/Defines.h>

#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/HashTypes.h"
#include "internal/datastructures/AABBTree.h"

namespace magique
{
    struct MapData final
    {
        const uint16_t* layerData[MAGIQUE_MAX_TILE_LAYERS]{};
        uint16_t width = 0;
        uint16_t height = 0;
        uint8_t layers = 0;
    };

    struct StaticCollisionData final
    {
        // Collision objects + manual colliders
        AABBTree aabbTree;
        HashMap<MapID, const std::vector<TileObject>*> mapObjects;

        // Tileset
        // hashset vs vec vs custom bitset
        vector<bool> tileCollisionVec;    // Holds information if the tile-index is solid
        const TileSet* tileSet = nullptr; // Only used for equality checks - global tileset
        vector<MapData> mapDataVec;       // Holds the begin pointers to each map

        // Manual colliders
        HashMap<int, vector<StaticCollider>> manualColliderMap; // Map as group can be anything

        [[nodiscard]] bool isSolid(const uint16_t tileNum) const
        {
            return tileNum < tileCollisionVec.size() && tileCollisionVec[tileNum];
        }
    };

    namespace global
    {
        inline StaticCollisionData STATIC_COLL_DATA{};
    }

} // namespace magique


#endif //STATICCOLLISIONDATA_H