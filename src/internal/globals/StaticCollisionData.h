#ifndef STATICCOLLISIONDATA_H
#define STATICCOLLISIONDATA_H

#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/HashTypes.h"
#include "internal/datastructures/MultiResolutionGrid.h"

namespace magique
{
    struct StaticCollisionData final
    {
        vector<StaticCollider> colliders;
        HashMap<MapID, const std::vector<TileObject>*> mapObjects;
        HashMap<MapID, SingleResolutionHashGrid<int, 32>> hashGrids;
        HashSet<uint16_t> collisionSet; // Holds information if the tile-index is solid
        TileMap* tileMap;
    };

    namespace global
    {
        inline StaticCollisionData STATIC_COLL_DATA{};
    }

} // namespace magique


#endif //STATICCOLLISIONDATA_H