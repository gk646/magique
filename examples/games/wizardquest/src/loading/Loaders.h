#ifndef LOADERS_H
#define LOADERS_H

#include <magique/core/Types.h>

struct TileLoader final : ITask<AssetPack>
{
    void execute(AssetPack& res) override;
};

struct TextureLoader final : ITask<AssetPack>
{
    void execute(AssetPack& assets) override;
};

struct EntityLoader final : ITask<AssetPack>
{
    void execute(AssetPack& res) override;
};


#endif // LOADERS_H