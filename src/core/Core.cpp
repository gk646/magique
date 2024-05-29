#include <magique/core/Core.h>

#include "core/CoreData.h"

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