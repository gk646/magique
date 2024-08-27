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
        if (tiles == -1)
        {
            LOG_ERROR("Failed to parse tileset: %s", asset.getFileName(true));
            return;
        }
        infoVec.reserve(tiles + 1);
        while (!XMLLineContainsTag(work, "tile"))
            cxstructs::str_skip_char(work, '\n', 1);
        while (!XMLLineContainsTag(work, "wangsets"))
        {
            TileInfo info;
            info.tileID = static_cast<uint16_t>(XMLGetValueInLine<int>(work, "id", UINT16_MAX));
            info.clazz = XMLGetValueInLine<int>(work, "type", INT32_MAX);
            info.probability = XMLGetValueInLine<float>(work, "probability", 1.0F);
            infoVec.push_back(info);
            cxstructs::str_skip_char(work,'\n',1);
        }
        const auto size = static_cast<int>(infoVec.size());
        LOG_INFO("Successfully loaded TileSet: %s | Marked Tiles: %d", asset.getFileName(true), size);
    }

    const std::vector<TileInfo>& TileSet::getTileInfo() const { return infoVec; }

} // namespace magique