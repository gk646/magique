#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <cstdint>

namespace magique
{
    template <typename... Resources>
    struct AssetManager final
    {
        // A tuple to hold the vectors
        std::tuple<std::vector<Resources>...> resourceVectors;

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

        // Function to move add a resource
        template <typename T>
        handle addResource(T&& resource)
        {
            auto& vec = getResourceVec<T>();
            auto handle = vec.size();
            vec.push_back(std::forward<T>(resource));
            return static_cast<enum class handle>(handle);
        }

        // Function to get a resource
        template <typename T>
        T& getResource(handle handle)
        {
            // If you get an error here you probably used a wrong handle or in the wrong method
            M_ASSERT(handle != handle::null, "Null handle!");
            M_ASSERT(getResourceVec<T>().size() > static_cast<int>(handle), "Cannot contain the resource");
            return getResourceVec<T>()[static_cast<uint32_t>(handle)];
        }
    };

    namespace global
    {
        inline AssetManager<TileSheet, Sound, Music, TextureRegion, SpriteSheet, TileMap, Playlist> ASSET_MANAGER;

    }

} // namespace magique
#endif //ASSETMANAGER_H