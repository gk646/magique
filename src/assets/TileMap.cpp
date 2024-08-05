#include <magique/assets/container/AssetContainer.h>
#include <magique/assets/types/TileMap.h>
#include <magique/util/Logging.h>

#include <raylib/raylib.h>
#include <cxutil/cxstring.h>
#include <magique/internal/Macros.h>

namespace magique
{
    constexpr unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
    constexpr unsigned FLIPPED_VERTICALLY_FLAG = 0x40000000;
    constexpr unsigned FLIPPED_DIAGONALLY_FLAG = 0x20000000;

    TileMap::TileMap(const Asset& asset)
    {
        auto ext = GetFileExtension(asset.path);

        if (ext == nullptr)
        {
            LOG_WARNING("No valid extension: %s", asset.path);
            return;
        }

        if (strcmp(ext, ".tmx") != 0)
        {
            LOG_WARNING("Not a valid tilemap extension: %s", asset.path);
            return;
        }

        auto workPtr = const_cast<char*>(asset.data); // keep user api const

        int layerWidth = -1;
        int layerHeight = -1;
        while (true)
        {
            cxstructs::str_skip_char(workPtr, '<', 1);
            if (strncmp(workPtr, "layer", cxstructs::str_len("layer")) == 0)
                break;
        }

        for (int i = 0; i < 15; ++i) // Limit in case we dont find
        {
            cxstructs::str_skip_char(workPtr, ' ', 1);
            if (strncmp(workPtr, "width", cxstructs::str_len("width")) == 0)
            {
                cxstructs::str_skip_char(workPtr, '"', 1);
                layerWidth = cxstructs::str_parse_int(workPtr);
            }

            if (strncmp(workPtr, "height", cxstructs::str_len("height")) == 0)
            {
                cxstructs::str_skip_char(workPtr, '"', 1);
                layerHeight = cxstructs::str_parse_int(workPtr);
            }

            if (layerWidth != -1 && layerHeight != -1)
                break;
        }

        if (layerWidth == -1 || layerHeight == -1)
        {
            LOG_WARNING("Could not load tilemap layer dimensions: %s", asset.path);
            return;
        }
        width = static_cast<int16_t>(layerWidth);
        height = static_cast<int16_t>(layerHeight);

        cxstructs::str_skip_char(workPtr, '\n', 2); // Skip to data start

        int accumulatedCount = 0;
        const int layerSize = layerWidth * layerHeight;

        for (int layer = 0; layer < MAGIQUE_MAX_TILEMAP_LAYERS; ++layer)
        {
            layerCount++;
            layerIndices[layer] = accumulatedCount;
            accumulatedCount += layerSize;
            // Reserve the layer
            layerData.reserve(accumulatedCount + 1);

            // right-down parsing
            int x = 0;
            for (int i = 0; i < layerSize - 1; ++i)
            {
                layerData.push_back(static_cast<uint16_t>(cxstructs::str_parse_int(workPtr)));
                x++;
                if (x == width)
                {
                    cxstructs::str_skip_char(workPtr, '\n', 1);
                    x = 0;
                }
                else
                    cxstructs::str_skip_char(workPtr, ',', 1);
            }
            // Push last
            layerData.push_back(static_cast<uint16_t>(cxstructs::str_parse_int(workPtr)));

            cxstructs::str_skip_char(workPtr, '\n', 3);
            cxstructs::str_skip_char(workPtr, '<', 1);
            if (strncmp(workPtr, "layer", 5) != 0)
            {
                break;
            }
        }
        LOG_INFO("Successfully loaded TileMap: %s | Layers: %d | Width: %d / Height %d ", asset.path, layerCount,
                 layerWidth, layerHeight);
    }

    uint16_t& TileMap::getTileNum(const int x, const int y, const int layer)
    {
        const auto start = getLayerStartIndex(layer);
        return layerData[start + y * width + x];
    }

    const uint16_t* TileMap::getLayerStart(const int layer) const { return &layerData[getLayerStartIndex(layer)]; }

    int TileMap::getLayerStartIndex(const int layer) const
    {
        M_ASSERT(layer < layerCount, "Layer out of bounds! 0-based indexing!");
        return static_cast<int>(layerIndices[layer]);
    }
} // namespace magique