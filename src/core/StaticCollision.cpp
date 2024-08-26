#include <raylib/raylib.h>

#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/TileSet.h>
#include <magique/core/StaticCollision.h>
#include <magique/util/Logging.h>

#include "internal/globals/StaticCollisionData.h"

namespace magique
{
    void SetStaticWorldBounds(const Rectangle& rectangle) { global::STATIC_COLL_DATA.worldBounds = rectangle; }

    void LoadMapColliders(const MapID map, const std::vector<TileObject>& collisionObjects, const float scale)
    {
        if (collisionObjects.empty())
            return;
        auto& data = global::STATIC_COLL_DATA;
        if (!data.objectReferences.objectVectors.contains(map))
        {
            data.objectReferences.objectVectors[map] = {}; // Initalize to nullptr
        }

        auto& ptrArr = data.objectReferences.objectVectors[map];
        for (const auto& ptr : ptrArr) // Check if the vector already exists
        {
            if (ptr == &collisionObjects)
            {
                return; // Skip if already exists
            }
        }

        int count = 0;
        for (auto& ptr : ptrArr) // Find insert position
        {
            if (ptr == nullptr)
            {
                ptr = &collisionObjects;
                break;
            }
            ++count;
        }
        if (count >= MAGIQUE_MAX_OBJECT_LAYERS)
        {
            LOG_WARNING("There are already the maximum amount of object vectors registered for this map");
            return;
        }

        // Insert
        auto& referenceVec = data.objectReferences.tileObjectMap[map];
        for (const auto& obj : collisionObjects)
        {
            if (!obj.visible) // Only visible objects
                continue;
            const float scaledX = obj.x * scale;
            const float scaledY = obj.y * scale;
            const float scaledWidth = obj.width * scale;
            const float scaledHeight = obj.height * scale;

            const auto num = data.objectStorage.insert(scaledX, scaledY, scaledWidth, scaledHeight);
            const auto staticID = StaticIDHelper::CreateID(num, obj.getClass());
            data.objectGrid.insert(staticID, scaledX, scaledY, scaledWidth, scaledHeight);
            referenceVec.push_back(num); // So we can uniquely delete later
        }
    }

    void SetGlobalTileSet(const TileSet& tileSet, const int collisionClass, const float tileSize) {}

    void LoadTileMap(const MapID map, const TileMap& tileMap, const std::initializer_list<int>& layers) {}

    void AddStaticColliderRect(const int group, float x, float y, float width, float height)
    {
        auto& data = global::STATIC_COLL_DATA;
        // data.manualColliderMap[group].push_back({x, y, width, height});
    }

    void RemoveColliderGroup(int group) {}


} // namespace magique