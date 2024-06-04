#include <magique/core/Core.h>

#include "core/CoreData.h"

#define CUTE_C2_IMPLEMENTATION
#include "external/raylib/src/rlgl.h"


#include <c2/cute_c2.h>

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


        return true;
    }



    void SetEntityUpdateDistance(int pixels) {}


    const vector<entt::entity>& GetUpdateEntities() { return LOGIC_TICK_DATA.entityUpdateVec; }


    DrawTickData& GetDrawTickData() { return DRAW_TICK_DATA; }


} // namespace magique