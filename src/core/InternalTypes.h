#ifndef INTERNALTYPES_H
#define INTERNALTYPES_H

#include <algorithm>

#include <magique/core/Types.h>
#include <magique/util/DataStructures.h>
#include <magique/core/Defines.h>
#include <magique/util/Logging.h>

#include <cxstructs/BitMask.h>
#include <raylib/raylib.h>

#include "core/datastructures/MultiResolutionGrid.h"

using CollisionPair = std::pair<entt::entity, entt::entity>;
struct PairHash
{
    std::size_t operator()(const CollisionPair& p) const
    {
        const uint64_t combined = static_cast<uint64_t>(p.first) << 32 | static_cast<uint64_t>(p.second);
        return std::hash<uint64_t>()(combined);
    }
};

namespace magique
{
    struct LogicTickData final
    {
        // Currently loaded map
        const Map* currentMap = nullptr;

        Pint cameraTilePos{};

        MapID currentZone;

        entt::entity camera;

        MapID loadedZones[MAGQIQUE_MAX_PLAYERS];

        // Change set for multiplayer events
        HashMap<entt::entity, cxstructs::EnumMask<UpdateFlag>> changedSet;

        // Cache for entities that are not in update range anymore
        // They are still updated for cache duration
        HashMap<entt::entity, uint16_t> entityUpdateCache;

        // vector containing the entites to update for this ticka
        vector<entt::entity> entityUpdateVec;

        // vector containing all entites to be drawn this tick
        // Culled with the camera
        vector<entt::entity> drawVec;

        // Contains entities that have been removed
        // This is for multiplayer queued updates
        vector<entt::entity> removedEntities;

        // Global hashGrid for all entities
        SingleResolutionHashGrid<entt::entity, 32> hashGrid{200};

        // Collects entities
        HashSet<entt::entity> collector;

        // Atomic spinlock - whenever and data is accessed on the draw thread
        std::atomic_flag flag;

        LogicTickData()
        {
            hashGrid.reserve(150, 1000);
            drawVec.reserve(1000);
            entityUpdateVec.reserve(1000);
            removedEntities.reserve(100);

            //Collision pairs
            collector.reserve(500);

            //MP Update set
            changedSet.reserve(1000);

            //Update cache
            entityUpdateCache.reserve(1000);
        }

        void lock()
        {
            while (flag.test_and_set(std::memory_order_acquire))
            {
            }
        }

        void unlock() { flag.clear(std::memory_order_release); }

        void clear()
        {
            changedSet.clear();
            entityUpdateCache.clear();
            entityUpdateVec.clear();
            drawVec.clear();
            removedEntities.clear();
            collector.clear();
            hashGrid.clear();
        }
    };

    struct Configuration final
    {
        // Toggles the performance overlay
        bool showPerformanceOverlay = true;

        // All logs visible
        util::LogLevel logLevel = util::LEVEL_NONE;

        // Update distance
        int entityUpdateDistance = 1000;

        // For how long entities in the cache are still updates after they are out of range
        uint16_t entityCacheDuration = 300; // 300 Ticks -> 5 seconds

        // Font
        Font engineFont;
    };

    template <typename... Resources>
    struct AssetManager final
    {
        // A tuple to hold the hash maps
        std::tuple<std::vector<Resources>...> resourceVectors;

        // Helper to get the index of a type in the parameter pack
        template <typename T, typename Tuple>
        struct Index;

        template <typename T, typename... Types>
        struct Index<T, std::tuple<T, Types...>>
        {
            static constexpr std::size_t value = 0;
        };

        template <typename T, typename U, typename... Types>
        struct Index<T, std::tuple<U, Types...>>
        {
            static constexpr std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
        };

        template <typename T>
        constexpr auto& getResourceVec()
        {
            return std::get<Index<T, std::tuple<Resources...>>::value>(resourceVectors);
        }

        // Function to add a resource
        template <typename T>
        handle addResource(const T& resource)
        {
            auto& vec = getResourceVec<T>();
            auto handle = vec.size();
            vec.push_back(resource);
            return static_cast<enum class handle>(handle);
        }

        // Function to get a resource
        template <typename T>
        T& getResource(handle handle)
        {
            // If you get an error here you probably used a handle in the wrong method
            // Or its just a null handle
            assert(getResourceVec<T>().size() > (int)handle && "Cannot contain the resource");
            return getResourceVec<T>()[static_cast<uint32_t>(handle)];
        }
    };


    // Uses naive 'scheduling' - if a sequence cant be deterministically described we skip to the next row
    // So if a spritesheet doesnt fit in the current row we just skip it and put it in the next wasting the space
    struct TextureAtlas final
    {
        int width = MAGIQUE_TEXTURE_ATLAS_WIDTH;   // Total width
        int height = MAGIQUE_TEXTURE_ATLAS_HEIGHT; // Total height
        int offX = 0;                              // Current offset from the top left
        int offY = 0;                              // Current offset from the top let
        unsigned int id = 0;                       // Texture id
        int currentStepHeight = 0;                 // Highest height of a texture in current row
        void* imageData = nullptr;                 // Save memory by only saving data ptr

        explicit TextureAtlas(Color color) // Just passed so its not auto constructed anywhere
        {
            // Always PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 and 1 mipmap
            const auto img = GenImageColor(width, height, color);
            imageData = img.data;
            const auto tex = LoadTextureFromImage(img);
            if (tex.id == 0)
            {
                LOG_ERROR("Failed to load texture atlas texture! No textures will work");
                UnloadImage(img);
            }
            id = tex.id;
        }

        TextureRegion addImage(const Image& image)
        {
            TextureRegion region = {0};
            if (offX + image.width > width)
            {
                offY += currentStepHeight;
                offX = 0;
                currentStepHeight = image.height;
                if (offY >= height)
                {
                    LOG_ERROR("Trying to add to a full texture atlas!");
                    return region;
                }
            }

            if (image.height > currentStepHeight) // Keep track of the highest image
                currentStepHeight = image.height;

            Image atlasImage = getImg();
            // Add the image
            ImageDraw(&atlasImage, image, {0, 0, (float)image.width, (float)image.height},
                      {(float)offX, (float)offY, (float)image.width, (float)image.height}, WHITE);

            UnloadImage(image);

            region.width = static_cast<uint16_t>(image.width);
            region.height = static_cast<uint16_t>(image.height);
            region.offX = static_cast<uint16_t>(offX);
            region.offY = static_cast<uint16_t>(offY);
            region.id = static_cast<uint16_t>(id);
            return region;
        }

        [[nodiscard]] Image getImg() const
        {
            Image img;
            img.data = imageData;
            img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            img.mipmaps = 1;
            img.width = width;
            img.height = height;
            return img;
        }

        void loadToGPU() const
        {
            // Always same format as image
            UpdateTexture({id, width, height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8}, imageData);
            UnloadImage(getImg());
        }
    };
} // namespace magique
#endif //INTERNALTYPES_H