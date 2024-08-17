#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/Asset.h>
#include <magique/internal/Macros.h>
#include <magique/util/Logging.h>

#include "internal/utils/XMLUtil.h"

namespace magique
{
    void ParseTileLayer(TileMap& tileMap, char*& data)
    {
        cxstructs::str_skip_char(data, '\n', 1);
        ASSERT(XMLLineContainsTag(data, "data"), "Layout Error: Failed to parse tile layer");
#if MAGIQUE_DEBUG == 1
        const auto val = XMLGetValueInLine<const char*>(data, "encoding", "nope");
        ASSERT(cxstructs::str_cmp_prefix(val, "csv"), "Tilemap has invalid encoding! Only supports csv");
#endif
        cxstructs::str_skip_char(data, '\n', 1);

        const auto addLayer = [](TileMap& map)
        {
            ASSERT(map.width != 0 && map.height != 0, "Internal Error: Dimensions not set!");

            const int tilePerLayer = map.width * map.height;
            const int newLayerSize = tilePerLayer * (map.layers + 1);

            auto* newData = new uint16_t[newLayerSize];
            if (map.tileData != nullptr)
            {
                std::memcpy(newData, map.tileData, tilePerLayer * map.layers * sizeof(uint16_t));
                delete[] map.tileData;
            }
            ++map.layers;
            map.tileData = newData;
        };
        const auto startIdx = tileMap.layers * (tileMap.width * tileMap.height);
        addLayer(tileMap);

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
            if (*data == '\n')
            {
                ++data;
            }
            else if (*data == '\r')
            {
                ++data;
                ++data;
            }
        }

        layerData[layerSize] = static_cast<uint16_t>(cxstructs::str_parse_int(data));
        cxstructs::str_skip_char(data, '\n', 3);
    }

    TileObject ParseObject(char* data)
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

    TileMap::TileMap(const Asset& asset)
    {
        auto* data = const_cast<char*>(asset.data); // keep user api const

        cxstructs::str_skip_char(data, '\n', 1); // Skip xml tag

        const auto mapWidth = XMLGetValueInLine<int>(data, "width", -1);
        const auto mapHeight = XMLGetValueInLine<int>(data, "height", -1);

        if (mapWidth == -1 || mapHeight == -1)
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

        width = static_cast<uint16_t>(mapWidth);
        height = static_cast<uint16_t>(mapHeight);

        for (int i = 0; i < MAGIQUE_MAX_TILE_LAYERS + MAGIQUE_MAX_OBJECT_LAYERS; ++i)
        {
            if (XMLLineContainsTag(data, "layer"))
            {
                ASSERT(XMLGetValueInLine<int>(data, "width", -1) == mapWidth, "Layers have different dimensions!");
                ASSERT(XMLGetValueInLine<int>(data, "height", -1) == mapHeight, "Layers have different dimensions!");
                ParseTileLayer(*this, data);
            }
            else if (XMLLineContainsTag(data, "objectgroup"))
            {
                ASSERT(objectLayers < MAGIQUE_MAX_OBJECT_LAYERS, "More object layers than configured!");
                cxstructs::str_skip_char(data, '\n', 1);
                while (!XMLLineContainsTag(data, "/objectgroup"))
                {
                    objectData[objectLayers].push_back(ParseObject(data));
                    cxstructs::str_skip_char(data, '\n', 1);
                }
                ++objectLayers;
                cxstructs::str_skip_char(data, '\n', 1);
            }else
            {
                break;
            }
        }

        LOG_INFO("Successfully loaded TileMap: %s | Layers: %d | Width/Height: %d / %d ", asset.path, layers, mapWidth,
                 mapHeight);
    }

    uint16_t& TileMap::getTileIndex(const int x, const int y, const int layer) { return tileData[layer]; }

    uint16_t TileMap::getTileIndex(const int x, const int y, const int layer) const
    {
        return getLayerData(layer)[x + y * width];
    }

    const uint16_t* TileMap::getLayerData(const int layer) const { return &tileData[width * height * layer]; }

    int TileMap::getWidth() const { return width; }
    int TileMap::getHeight() const { return height; }
    int TileMap::getLayerCount() const { return layers; }

    std::vector<TileObject>& TileMap::getObjects(const int layer) { return objectData[layer]; }

} // namespace magique