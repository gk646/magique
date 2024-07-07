#include <entt/entity/entity.hpp>
#include <magique/core/Particles.h>

#include <raylib/raylib.h>

namespace magique
{
    void CreateScreenParticle(const ScreenEmitter& emitter) {}

    entt::entity CreateEntityParticle(const EntityEmitter& emitter) { return entt::null; }

    //----------------- EMITTER -----------------//

    EmitterBase& EmitterBase::setEmissionPosition(float x, float y)
    {
        data.x = x;
        data.y = y;
        return *this;
    }

    EmitterBase& EmitterBase::setAmount(int amount)
    {
        data.amount = amount;
        return *this;
    }

    EmitterBase& EmitterBase::setColor(const Color& color)
    {
        data.r = color.r;
        data.g = color.g;
        data.b = color.b;
        data.a = color.a;
        return *this;
    }

    EmitterBase& EmitterBase::setColorFunction(ColorFunction func)
    {
        data.colorFunc = func;
        return *this;
    }


    EmitterBase& EmitterBase::setShape(Shape shape)
    {
        data.shape = shape;
        return *this;
    }

    EmitterBase& EmitterBase::setDimensions(float w, float h)
    {
        data.width = w;
        data.height = h;
        return *this;
    }

    EmitterBase& EmitterBase::setLifetime(int val)
    {
        data.lifeTime = val;
        return *this;
    }

    EmitterBase& EmitterBase::setDirection(float x, float y)
    {
        data.dirX = x;
        data.dirY = y;
        return *this;
    }

    EmitterBase& EmitterBase::setMinScale(float val)
    {
        data.minScale = val;
        return *this;
    }

    EmitterBase& EmitterBase::setMaxScale(float val)
    {
        data.maxScale = val;
        return *this;
    }

    EmitterBase& EmitterBase::setScaleFunction(ScaleFunction func)
    {
        data.scaleFunc = func;
        return *this;
    }
    EmitterBase& EmitterBase::setSpread(float val)
    {
        data.spreadAngle = val;
        return *this;
    }

    EmitterBase& EmitterBase::setMinInitialVelocity(float val)
    {
        data.minInitVeloc = val;
        return *this;
    }

    EmitterBase& EmitterBase::setMaxInitialVelocity(float val)
    {
        data.maxInitVeloc = val;
        return *this;
    }

    EmitterBase& EmitterBase::setEmissionShape(Shape val)
    {
        data.emissionShape = val;
        return *this;
    }

    EmitterBase& EmitterBase::setEmissionShapeDimensions(float w, float h)
    {
        data.emWidth = w;
        data.emHeight = h;
        return *this;
    }

    EmitterBase& EmitterBase::setResolutionScaling(bool val)
    {
        data.resolutionScaling = val;
        return *this;
    }

    EmitterBase::ScaleFunction EmitterBase::GetSmoothStep()
    {
        return [](const float scale, const float t) -> float { return scale * (t * t * (3 - 2 * t)); };
    }

} // namespace magique