#include <magique/core/Core.h>

#include "core/CoreData.h"

#define CUTE_C2_IMPLEMENTATION
#include <c2/cute_c2.h>

namespace magique
{

    void core::SetEntityUpdateDistance(int pixels)
    {

    }


    const vector<entt::entity>& core::GetUpdateEntities()
    {
       return LOGIC_TICK_DATA.entityUpdateVec;
    }


    DrawTickData& core::GetDrawTickData()
    {
      return DRAW_TICK_DATA;
    }


} // namespace magique