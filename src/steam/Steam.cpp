#include <steam/steam_api.h>

void CreateAppIDFile()
{
    auto* filename = "steam_appid.txt";
    std::ofstream file(filename);

    if (file.is_open())
    {
        file << "480";
        file.close();
    }
    else
    {
        LOG_ERROR("Unable to create steam_appid.txt file with test id 480 - Do it manually!");
    }
}
namespace magique{

    bool InitSteam(
){
#if MAGIQUE_DEBUG == 1
        CreateAppIDFile();
#endif

        SteamErrMsg errMsg;
        if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK)
        {
            LOG_ERROR(errMsg);
            return false;
        }
        LOG_INFO("Successfully initialized multiplayer");
        return true;
}
}