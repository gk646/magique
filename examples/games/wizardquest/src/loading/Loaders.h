#ifndef LOADERS_H
#define LOADERS_H

#include <magique/core/Types.h>

using namespace magique;

struct TileLoader final : ITask<AssetContainer>
{
    void execute(AssetContainer& res) override;
};

struct TextureLoader final : ITask<AssetContainer>
{
    void execute(AssetContainer& assets) override;
};

struct EntityLoader final : ITask<AssetContainer>
{
    void execute(AssetContainer& res) override;
};


#endif // LOADERS_H