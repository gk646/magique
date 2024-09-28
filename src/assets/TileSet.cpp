#include <cxutil/cxstring.h>

#include <magique/assets/types/TileSet.h>
#include <internal/utils/XMLUtil.h>
#include <magique/assets/types/Asset.h>
#include <magique/util/Logging.h>

namespace magique
{
    TileSet::TileSet(const Asset& asset)
    {
        char* work = const_cast<char*>(asset.data);
        cxstructs::str_skip_char(work, '\n', 1); // Skip xml tag

        const auto tiles = XMLGetValueInLine<int>(work, "tilecount", -1);
        const auto tileWidth = XMLGetValueInLine<int>(work, "tilewidth", -1);
        const auto tileHeight = XMLGetValueInLine<int>(work, "tileheight", -1);
        if (tiles == -1 || tileWidth == -1 || tileHeight == -1)
        {
            LOG_ERROR("Failed to parse tileset: %s", asset.getFileName(true));
            return;
        }
        if (tileHeight != tileWidth)
        {
            LOG_ERROR("Only square tiles supported: %s", asset.getFileName(true));
            return;
        }
        tileSize = tileWidth;
        infoVec.reserve(tiles + 1);

        // Skip to tile info
        while (!XMLLineContainsTag(work, "tile"))
            cxstructs::str_skip_char(work, '\n', 1);

        // Parse tile info
        while (XMLLineContainsTag(work, "tile"))
        {
            TileInfo info;
            info.tileID = static_cast<uint16_t>(XMLGetValueInLine<int>(work, "id", UINT16_MAX));
            info.clazz = XMLGetValueInLine<int>(work, "type", INT32_MAX);
            if (!XMLLineContainsCloseTag(work)) // Contains collision info
            {
                cxstructs::str_skip_char(work, '\n', 1); // Skip tile tag
                cxstructs::str_skip_char(work, '\n', 1); // Skip objectgroup tag
                info.x = XMLGetValueInLine<uint8_t>(work, "x", UINT8_MAX);
                info.y = XMLGetValueInLine<uint8_t>(work, "y", UINT8_MAX);
                info.width = XMLGetValueInLine<uint8_t>(work, "width", UINT8_MAX);
                info.height = XMLGetValueInLine<uint8_t>(work, "height", UINT8_MAX);
                cxstructs::str_skip_char(work, '\n', 1); // Skip objectgroup close tag
                cxstructs::str_skip_char(work, '\n', 1); // Skip tile close tag
#ifdef MAGIQUE_DEBUG
                if (info.x == UINT8_MAX || info.y == UINT8_MAX || info.width == UINT8_MAX ||
                    info.height == UINT8_MAX)
                {
                    LOG_WARNING("Failed to parse tile collision area: %s", asset.getFileName(true));
                }
#endif
            }
            cxstructs::str_skip_char(work, '\n', 1); // Skip tile tag open
#ifdef MAGIQUE_DEBUG
            if (info.tileID == UINT16_MAX || info.clazz == INT32_MAX)
            {
                LOG_WARNING("Failed to parse tile info: %s", asset.getFileName(true));
            }
#endif
            infoVec.push_back(info);
        }
        const auto size = static_cast<int>(infoVec.size());
        LOG_INFO("Successfully loaded TileSet: %s | Marked Tiles: %d", asset.getFileName(true), size);
    }

    const std::vector<TileInfo>& TileSet::getTileInfo() const { return infoVec; }

    int TileSet::getTileSize() const { return tileSize; }

} // namespace magique