#include <magique/core/Core.h>

#include "core/CoreData.h"

static bool initCalled = false;
namespace magique
{
    bool InitMagique()
    {
        if (initCalled)
        {
            LOG_WARNING("Init called twice. Skipping...");
            return true;
        }
        initCalled = true;

        for (int i = 0; i < CUSTOM_2 + 1; ++i)
        {
            TEXTURE_ATLASES.emplace_back(BLANK);
        }

        CONFIGURATION.engineFont = GetFontDefault();
        if (CONFIGURATION.engineFont.texture.id == 0)
        {
            LOG_ERROR("Failed to load default font");
            return false;
        }
        return true;
    }


    void SyncThreads() { LOGIC_TICK_DATA.lock(); }


    void UnSyncThreads() { LOGIC_TICK_DATA.unlock(); }


    void SetEntityUpdateDistance(int pixels) {}


    const vector<entt::entity>& GetUpdateEntities() { return LOGIC_TICK_DATA.entityUpdateVec; }


    DrawTickData& GetDrawTickData() { return DRAW_TICK_DATA; }


} // namespace magique