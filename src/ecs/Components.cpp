#include <magique/ecs/Components.h>
#include <magique/util/Logging.h>

bool isValidLayer(magique::CollisionLayer layer)
{
    const auto layerNum = static_cast<uint8_t>(layer);
    return layerNum != 0 && (layerNum & layerNum - 1) == 0;
}

void CollisionC::removeAllLayers() { layerMask = 0; }

void CollisionC::addCollisionLayer(magique::CollisionLayer layer)
{
    if (isValidLayer(layer))
    {
        layerMask |= static_cast<uint8_t>(layer);
    }
    else
    {
        LOG_WARNING("Trying to assign invalid collision layer! Skipping");
    }
}

void CollisionC::removeCollisionLayer(magique::CollisionLayer layer)
{
    if (isValidLayer(layer))
    {
        layerMask &= ~static_cast<uint8_t>(layer);
    }
    else
    {
        LOG_WARNING("Trying to assign invalid collision layer! Skipping");
    }
}