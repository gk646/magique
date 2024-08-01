#include <raylib/raylib.h>

#include <entt/entity/entity.hpp>
#include <magique/core/Particles.h>

#include "internal/globals/ParticleData.h"

namespace magique
{

    void CreateScreenParticle(const ScreenEmitter& emitter)
    {
        ScreenParticle particle{};
        const auto& data = emitter.data;

        // Base
        particle.r = data.r;
        particle.g = data.g;
        particle.b = data.b;
        particle.a = data.a;

        particle.age = 0;
        particle.lifetime = data.lifeTime;

        // Spawn position
        switch (data.emShape)
        {
        case Shape::RECT:
            particle.x = GetRandomValue(data.emX, data.p1);
            particle.y = GetRandomValue(data.emY, data.p2);
            break;
        case Shape::CIRCLE:
            const float angle = GetRandomValue(0, 360);
            const float dist = static_cast<float>(GetRandomValue(0, 100)) / 100.0F;
            const float angle_rad = angle * (PI / 180.0f);
            particle.x = dist * cos(angle_rad);
            particle.y = dist * sin(angle_rad);
            break;
        case Shape::TRIANGLE:
            break;
        }

        const float p = static_cast<float>(GetRandomValue(0, 100)) / 100.0F;
        particle.scale = (data.maxScale - data.minScale) * p;

        global::PARTICLE_DATA.addParticle(particle);
    }

    entt::entity CreateEntityParticle(const EntityEmitter& emitter) { return entt::null; }

    //----------------- EMITTER -----------------//

    EmitterBase& EmitterBase::setEmissionPosition(const float x, const float y)
    {
        data.emissionX = x;
        data.emissionY = y;
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

    EmitterBase& EmitterBase::setMinScale(const float val)
    {
        data.minScale = val;
        return *this;
    }

    EmitterBase& EmitterBase::setMaxScale(const float val)
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

    EmitterBase& EmitterBase::setResolutionScaling(const bool val)
    {
        data.resolutionScaling = val;
        return *this;
    }

    EmitterBase::ScaleFunction EmitterBase::GetSmoothStep()
    {
        return [](const float scale, const float t) -> float { return scale * (t * t * (3 - 2 * t)); };
    }


} // namespace magique