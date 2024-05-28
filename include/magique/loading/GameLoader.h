#ifndef GAMELOADER_H
#define GAMELOADER_H

#include "taskexecutor/TaskExecutor.h"

struct AssetContainer;
namespace magique
{
    struct GameLoader final : TaskExecutor<AssetContainer>
    {
        auto load() -> bool override;
    };

} // namespace magique

#endif //GAMELOADER_H