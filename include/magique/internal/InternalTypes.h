// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_INTERNAL_TYPES_H
#define MAGIQUE_INTERNAL_TYPES_H

#include <cstring>
#include <vector>
#include <magique/core/Types.h>

//===============================================
// Public Internal Module
//===============================================
// .....................................................................
// Internal but public types or methods. Don't modify or use them (unless you know what your doing).
// They have to be public due to templates (or other reasons) and are used internally
// .....................................................................

namespace magique::internal
{

    struct StorageContainer
    {
        StorageContainer() = default;
        StorageContainer(const StorageContainer& other) = delete; // Involves potentially copying a lot of data
        StorageContainer&
        operator=(const StorageContainer& other) = delete; // Involves potentially copying a lot of data
        StorageContainer(StorageContainer&& other) noexcept = default;
        StorageContainer& operator=(StorageContainer&& other) noexcept = default;
        ~StorageContainer(); // Will clean itself up automatically

    protected:
        MQ_MAKE_PUB()
        static bool ToFile(StorageContainer& storage, std::string_view path, std::string_view name, EncryptionKey key);
        static bool FromFile(StorageContainer& storage, std::string_view path, std::string_view name, EncryptionKey key);

        // Erases the storage with the given id
        void eraseImpl(std::string_view slot);
        // Erases all storage slots
        void clearImpl() { cells.clear(); }

        StorageType getSlotTypeImpl(std::string_view slot)
        {
            const auto* cell = getCell(slot);
            if (cell == nullptr)
            {
                return StorageType::EMPTY;
            }
            return cell->type;
        }

        StorageCell* getCell(std::string_view slot)
        {
            for (auto& cell : cells)
            {
                if (cell.name == slot)
                {
                    return &cell;
                }
            }
            return nullptr;
        }

        StorageCell& getCellOrNew(std::string_view slot, StorageType type)
        {
            auto* cell = getCell(slot);
            if (cell == nullptr)
            {
                cell = &cells.emplace_back(type);
                cell->name = slot;
                return *cell;
            }
            cell->type = type;
            return *cell;
        }

        void assignDataImpl(std::string_view slot, const void* data, const int bytes, const StorageType type)
        {
            auto& cell = getCellOrNew(slot, type);
            cell.data.resize(bytes);
            std::memcpy(cell.data.data(), data, bytes);
        }

        std::vector<StorageCell> cells;
        bool isLoaded = false;
        bool isSaved = false;
    };

    struct EmitterData final
    {
        std::vector<WeightedColor> colors{{RED}};
        using ScaleFunction = float (*)(float s, float t);
        using ColorFunction = Color (*)(const Color& c, float t);

        ScaleFunction scaleFunc = nullptr;
        ColorFunction colorFunc = nullptr;
        void* tickFunc = nullptr;
        mutable Point emissionPos{0, 0}; // RECT: width/height  / CIRCLE: radius
        Point emissionDims{1, 1};        // RECT: width/height  / CIRCLE: radius
        Point emissionAnchor{};
        Point particleDims{1};
        Point lifeTime{1};
        Point scale{1};
        Point veloc{1};
        Point angularVelocity{0, 0};
        Point direction{0, -1}; // Direction
        Point gravity;          // Gravity in x and y direction
        float rotation = 0;     // Rotation around the anchor
        float volume = 1.0F;    // How much of the body will be treated as spawnable area
        float spreadAngle = 0;  // Spread angle around the direction
        float angularGravity = 0.0F;
        Shape shape = Shape::RECT;
        Shape emShape = Shape::TRIANGLE; // Default is point emission
    };

    struct SceneManagerMapping final
    {
        std::string name;
        UIObject* object = nullptr;
    };

    struct PacketTypeStats
    {
        MessageType type;
        float count;   // How many times it was sent
        float size;    // Size of a single packet
        float total;   // Total size in bytes
        float contrib; // Contribution to total in %
    };

    struct MultiplayerStatsData
    {
        std::vector<PacketTypeStats> incoming;
        std::vector<PacketTypeStats> outgoing;
        float bytesIn;
        float bytesOut;
    };


} // namespace magique::internal

#endif // MAGIQUE_INTERNAL_TYPES_H
