// SPDX-License-Identifier: zlib-acknowledgement
#include <cstring>
#include <magique/assets/types/TileSet.h>
#include <magique/util/Logging.h>

namespace magique
{
    const std::vector<TileInfo>& TileSet::getTilesInfo() const { return infoVec; }

    int TileSet::getTileSize() const { return tileSize; }

    TileInfo& TileSet::getInfo(const uint8_t tileId)
    {
        for (auto& info : infoVec)
        {
            if (info.tileID == tileId)
            {
                return info;
            }
        }
        LOG_ERROR("No tile with the given id found");
        return infoVec.front();
    }

    const TileInfo* TileSet::getInfo(const uint8_t tileId) const
    {
        for (const auto& info : infoVec)
        {
            if (info.tileID == tileId)
            {
                return &info;
            }
        }
        return nullptr;
    }

    const TileInfo* TileSet::getInfo(std::string_view image) const
    {
        for (const auto& info : infoVec)
        {
            if (info.image == image)
            {
                return &info;
            }
        }
        return nullptr;
    }

    int TileSet::getTileCount() const { return tileCount; }

    const std::vector<TileAnimation>& TileSet::getAnimations() const { return animations; }

} // namespace magique
