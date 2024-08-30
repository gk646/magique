#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/Asset.h>
#include <magique/internal/Macros.h>
#include <magique/util/Logging.h>

#include <cstddef>

#include "internal/utils/XMLUtil.h"

namespace magique
{
    void ParseTileLayer(TileMap& tileMap, char*& data)
    {
        MAGIQUE_ASSERT(tileMap.width != 0 && tileMap.height != 0, "Internal Error: Dimensions not set!");

        cxstructs::str_skip_char(data, '\n', 1);
        MAGIQUE_ASSERT(XMLLineContainsTag(data, "data"), "Layout Error: Failed to parse tile layer");
#ifdef MAGIQUE_DEBUG
        const auto val = XMLGetValueInLine<const char*>(data, "encoding", "nope");
        MAGIQUE_ASSERT(cxstructs::str_cmp_prefix(val, "csv"), "Tilemap has invalid encoding! Only supports csv");
#endif
        cxstructs::str_skip_char(data, '\n', 1);

        const auto tilesPerLayer = static_cast<const size_t>(tileMap.width * tileMap.height);
        const auto startIdx = tileMap.layers * tilesPerLayer;

        const size_t newDataSize = startIdx + tilesPerLayer;
        auto* newData = new uint16_t[newDataSize];
        if (tileMap.tileData != nullptr)
        {
            std::memcpy(newData, tileMap.tileData, startIdx * sizeof(uint16_t));
            delete[] tileMap.tileData;
        }
        ++tileMap.layers;
        tileMap.tileData = newData;

        auto* layerData = &tileMap.tileData[startIdx];
        const auto layerSize = tileMap.width * tileMap.height - 1;

        // right-down parsing
        for (int i = 0; i < layerSize; ++i)
        {
            layerData[i] = static_cast<uint16_t>(cxstructs::str_parse_int(data));
            while (*data != ',')
            {
                ++data;
            }
            ++data;
            if (*data == '\n' || *data == '\r')
            {
                if (*data == '\r')
                {
                    ++data;
                }
                ++data;
            }
        }

        layerData[layerSize] = static_cast<uint16_t>(cxstructs::str_parse_int(data));
        cxstructs::str_skip_char(data, '\n', 3);
    }

    TileObject ParseObject(const char* data)
    {
        TileObject object;
        object.id = XMLGetValueInLine<int>(data, "id", -1);

        const char* name = XMLGetValueInLine<const char*>(data, "name", nullptr);
        if (name != nullptr)
        {
            int i = 0;
            while (name[i] != '"' && name[i] != '\0')
                ++i;
            object.name = new char[i + 1];
            std::memcpy(object.name, name, i);
            object.name[i] = '\0';
        }

        object.type = XMLGetValueInLine<int>(data, "type", -1);
        object.x = XMLGetValueInLine<float>(data, "x", 0);
        object.y = XMLGetValueInLine<float>(data, "y", 0);
        object.width = XMLGetValueInLine<float>(data, "width", 0);
        object.height = XMLGetValueInLine<float>(data, "height", 0);
        object.visible = XMLGetValueInLine<int>(data, "visible", 1) == 1;
        return object;
    }

    //-------------- TILEMAP --------------

    TileMap::TileMap(const Asset& asset)
    {
        auto* data = const_cast<char*>(asset.data); // keep user api const
        cxstructs::str_skip_char(data, '\n', 1);    // Skip xml tag

        width = XMLGetValueInLine<uint16_t>(data, "width", UINT16_MAX);
        height = XMLGetValueInLine<uint16_t>(data, "height", UINT16_MAX);

        if (width == -1 || height == -1)
        {
            LOG_ERROR("Could not load tilemap layer dimensions: %s", asset.path);
            return;
        }

        const auto* renderOrder = XMLGetValueInLine<const char*>(data, "renderorder", "nope");
        if (!cxstructs::str_cmp_prefix(renderOrder, "right-down"))
        {
            LOG_ERROR("Only right-down renderorder supported!");
            return;
        }
        cxstructs::str_skip_char(data, '\n', 2); // Next line

        for (int i = 0; i < MAGIQUE_MAX_TILE_LAYERS + MAGIQUE_MAX_OBJECT_LAYERS; ++i)
        {
            if (XMLLineContainsTag(data, "layer"))
            {
                const auto* msg = "Layers have different dimensions!";
                MAGIQUE_ASSERT(XMLGetValueInLine<int>(data, "width", UINT16_MAX) == width, msg);
                MAGIQUE_ASSERT(XMLGetValueInLine<int>(data, "height", UINT16_MAX) == height, msg);
                ParseTileLayer(*this, data);
            }
            else if (XMLLineContainsTag(data, "objectgroup"))
            {
                MAGIQUE_ASSERT(objectLayers < MAGIQUE_MAX_OBJECT_LAYERS, "More object layers than configured!");
                if (!XMLLineContainsCloseTag(data)) // Detect empty layer
                {
                    cxstructs::str_skip_char(data, '\n', 1);
                    while (!XMLLineContainsTag(data, "/objectgroup"))
                    {
                        objectData[objectLayers].push_back(ParseObject(data));
                        cxstructs::str_skip_char(data, '\n', 1);
                    }
                }
                else // Empty object layer
                {
                    LOG_WARNING("Empty object layer detected. This is likely a oversight: %s", asset.path);
                }
                ++objectLayers;
                cxstructs::str_skip_char(data, '\n', 1);
            }
            else
            {
                break;
            }
        }
        const auto* msg = "Successfully loaded TileMap: %s | Layers: %d | Width/Height: %d / %d ";
        LOG_INFO(msg, asset.path, layers, width, height);
    }

    uint16_t& TileMap::getTileIndex(const int x, const int y, const int layer)
    {
        return tileData[width * height * layer + x + y * width];
    }

    uint16_t TileMap::getTileIndex(const int x, const int y, const int layer) const
    {
        return getLayerData(layer)[x + y * width];
    }

    const uint16_t* TileMap::getLayerData(const int layer) const
    {
        MAGIQUE_ASSERT(layer < layers, "TileMap does not have that many tile layers");
        return &tileData[width * height * layer];
    }

    int TileMap::getWidth() const { return width; }
    int TileMap::getHeight() const { return height; }
    int TileMap::getLayerCount() const { return layers; }

    std::vector<TileObject>& TileMap::getObjects(const int layer)
    {
        MAGIQUE_ASSERT(layer < objectLayers, "TileMap does not have that many object layers");
        return objectData[layer];
    }

} // namespace magique