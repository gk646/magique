#ifndef MAGIQUE_STEAMDATA_H
#define MAGIQUE_STEAMDATA_H

#include <fstream>
#if MAGIQUE_USE_STEAM == 0
#error "Using Steam features without enabling it! To enable Steam use CMake: MAGIQUE_STEAM"
#else
#include <steam/steam_api.h>
#endif

namespace magique
{
    struct SteamData final
    {
        CSteamID userID{};

        SteamData()
        {
            constexpr auto* filename = "steam_appid.txt";
            constexpr auto* id = "480";

            const std::ifstream file(filename);
            if (!file.good())
            {
                const auto newFile = fopen(filename, "wb");
                if (newFile != nullptr)
                {
                    fwrite(id, 3, 1, newFile);
                    fclose(newFile);
                }
                else
                {
                    LOG_ERROR("Unable to create steam_appid.txt file with test id 480 - Do it manually!");
                }
            }

            SteamErrMsg errMsg;
            if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK)
            {
                LOG_ERROR(errMsg);
                return;
            }
            LOG_INFO("Successfully initialized steam");
        }

        void close()
        {
            SteamAPI_RunCallbacks(); // Run callbacks one more time - flushing
            SteamAPI_Shutdown();
        }

        void update() { SteamAPI_RunCallbacks(); }
    };

    namespace global
    {
        inline SteamData STEAM_DATA{};
    }
} // namespace magique

#endif //MAGIQUE_STEAMDATA_H