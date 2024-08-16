#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/Asset.h>
#include <magique/internal/Macros.h>
#include <magique/util/Logging.h>

#include "internal/headers/XMLUtil.h"

namespace magique
{
    void ParseTileLayer(TileMap& tileMap, char* data)
    {
        cxstructs::str_skip_char(data, '\n', 1);
        ASSERT(XMLLineContainsTag(data, "data"), "Layout Error: Failed to parse tile layer");
#if MAGIQUE_DEBUG == 1
        const auto val = XMLGetValueInLine<const char*>(data, "encoding");
        ASSERT(val != nullptr, "Tilemap has invalid encoding! Only supports csv");
        ASSERT(strncmp(val, "csv", 3) == 0, "Tilemap has invalid encoding! Only supports csv");
#endif
        cxstructs::str_skip_char(data, '\n', 1);

        const auto addLayer = [](TileMap& map)
        {
            ASSERT(map.width != 0 && map.height != 0, "Internal Error: Dimensions not set!");

            const int totalTiles = map.width * map.height;
            const int newLayerSize = totalTiles * (map.layers + 1);

            auto* newData = new uint16_t[newLayerSize]{};

            if (map.tileData != nullptr)
            {
                std::memcpy(newData, map.tileData, newLayerSize * sizeof(uint16_t));
            }
            ++map.layers;
            delete[] map.tileData;
            map.tileData = newData;
        };
        addLayer(tileMap);

        const auto startIdx = tileMap.layers * (tileMap.width * tileMap.height);
        auto* layerData = &tileMap.tileData[startIdx];
        const auto layerSize = tileMap.width * tileMap.height;

        // right-down parsing
        for (int i = 0; i < layerSize; ++i)
        {
            layerData[i] = static_cast<uint16_t>(cxstructs::str_parse_int(data));
            ++data;
            if (*data == '\n')
            {
                ++data;
            }
        }
    }

    TileObject ParseObject(char* data)
    {
        TileObject object;
        object.id = XMLGetValueInLine<int>(data, "id");

        const char* name = XMLGetValueInLine<const char*>(data, "name");
        if (name != nullptr)
        {
            int i = 0;
            while (name[i] != '"' && name[i] != '\0')
                ++i;
            object.name = new char[i + 1];
            std::memcpy(object.name, name, i);
            object.name[i] = '\0';
        }

        object.type = XMLGetValueInLine<int>(data, "type");
        object.x = XMLGetValueInLine<float>(data, "x");
        object.y = XMLGetValueInLine<float>(data, "y");
        object.width = XMLGetValueInLine<float>(data, "width");
        object.height = XMLGetValueInLine<float>(data, "height");
        object.visible = XMLGetValueInLine<int>(data, "visible") == 1;
        return object;
    }

    void ParseObjectLayer(std::vector<TileObject>& objects, char* data) {}

    TileMap::TileMap(const Asset& asset)
    {
        auto* data = const_cast<char*>(asset.data); // keep user api const

        int mapWidth = -1;
        int mapHeight = -1;
        cxstructs::str_skip_char(data, '\n', 1); // Skip xml tag

        mapWidth = XMLGetValueInLine<int>(data, "width");
        mapHeight = XMLGetValueInLine<int>(data, "height");

        if (mapWidth == INT32_MIN || mapHeight == INT32_MIN)
        {
            LOG_ERROR("Could not load tilemap layer dimensions: %s", asset.path);
            return;
        }

        const auto* renderOrder = XMLGetValueInLine<const char*>(data, "renderorder");
        if (renderOrder == nullptr || strcmp(renderOrder, "right-down") != 0)
        {
            LOG_ERROR("Only right-down renderorder supported!");
            return;
        }

        width = static_cast<uint16_t>(mapWidth);
        height = static_cast<uint16_t>(mapHeight);

        for (int i = 0; i < MAGIQUE_MAX_TILE_LAYERS + MAGIQUE_MAX_OBJECT_LAYERS; ++i)
        {
            cxstructs::str_skip_char(data, '\n', 1); // Next line
            if (XMLLineContainsTag(data, "layer"))
            {
                ASSERT(XMLGetValueInLine<int>(data, "width") == mapWidth, "Layers have different dimensions!");
                ASSERT(XMLGetValueInLine<int>(data, "height") == mapHeight, "Layers have different dimensions!");
                ParseTileLayer(*this, data);
            }
            else if (XMLLineContainsTag(data, "objectgroup"))
            {
                ASSERT(objectLayers < MAGIQUE_MAX_OBJECT_LAYERS, "More object layers than configured!");
                while (!XMLLineContainsTag(data, "/objectgroup"))
                {
                    objectData[objectLayers].push_back(ParseObject(data));
                    cxstructs::str_skip_char(data, '\n', 1);
                }
                ++objectLayers;
            }
        }

        LOG_INFO("Successfully loaded TileMap: %s | Layers: %d | Width: %d / Height %d ", asset.path, layers, mapWidth,
                 mapHeight);
    }

    uint16_t& TileMap::getTileIndex(const int x, const int y, const int layer)
    {
        return tileData[layer];
    }

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