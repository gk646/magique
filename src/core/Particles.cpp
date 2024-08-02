#include <cmath>
#include <raylib/raylib.h>

#include <entt/entity/entity.hpp>
#include <magique/core/Particles.h>

#include "internal/globals/ParticleData.h"

#include <magique/core/Core.h>

namespace magique
{
    void DrawParticles() { global::PARTICLE_DATA.render(); }

    void CreateScreenParticle(const ScreenEmitter& emitter, const int amount)
    {
        const auto& data = emitter.data;
        for (int i = 0; i < amount; ++i)
        {
            ScreenParticle particle;
            particle.emitter = &emitter;

            // x,y
            switch (data.emShape) //  No triangle emission shape
            {
            case Shape::RECT:
                particle.x = GetRandomValue(data.emX, data.emp1);
                particle.y = GetRandomValue(data.emY, data.emp2);
                break;
            case Shape::CIRCLE:
                {
                    const float angle = static_cast<float>(GetRandomValue(0, 360)) * (PI / 180.0f);
                    const float dist = data.emp1 * sqrt(static_cast<float>(GetRandomValue(0, 100)) / 100.0F);
                    particle.x = dist * cos(angle);
                    particle.y = dist * sin(angle);
                }
                break;
            case Shape::CAPSULE: // Acts as point type
                particle.x = data.emX;
                particle.y = data.emY;
                break;
            }
            // Higher quality randomness should be worth it - and the random is pretty fast
            const float p = static_cast<float>(GetRandomValue(0, 100)) / 100.0F;
            const float p1 = static_cast<float>(GetRandomValue(0, 100)) / 100.0F;

            // Scale
            particle.scale = data.minScale + (data.maxScale - data.minScale) * p1;

            // p1,p2,p3,p4
            particle.p1 = static_cast<int16_t>(std::round(data.p1 * particle.scale));
            particle.p2 = static_cast<int16_t>(std::round(data.p2 * particle.scale));
            particle.p3 = static_cast<int16_t>(std::round(data.p3 * particle.scale));
            particle.p4 = static_cast<int16_t>(std::round(data.p4 * particle.scale));

            // Spread
            float dirX = data.dirX;
            float dirY = data.dirY;
            if (data.spreadAngle > 0)
            {
                const float p2 = static_cast<float>(GetRandomValue(0, 100)) / 100.0F;
                const float spreadAngle = (-data.spreadAngle / 2.0F + data.spreadAngle * p2) * DEG2RAD;
                const float currentAngle = atan2(data.dirY, data.dirX);
                const float newAngle = currentAngle + spreadAngle;
                dirX = cos(newAngle);
                dirY = sin(newAngle);
            }

            // vx,vy - velocity
            const float velocity = data.minInitVeloc + (data.maxInitVeloc - data.minInitVeloc) * p;
            particle.vx = velocity * dirX;
            particle.vy = velocity * dirY;


            // Rest
            particle.age = 0;
            particle.shape = data.shape;
            particle.r = data.r;
            particle.g = data.g;
            particle.b = data.b;
            particle.a = data.a;

            SyncThreads();
            global::PARTICLE_DATA.addParticle(particle);
            UnSyncThreads();
        }
    }

    entt::entity CreateEntityParticle(const EntityEmitter& emitter, const int amount) { return entt::null; }

    //----------------- EMITTER -----------------//

    EmitterBase& EmitterBase::setEmissionPosition(const float x, const float y)
    {
        data.emX = x;
        data.emY = y;
        return *this;
    }

    EmitterBase& EmitterBase::setEmissionShape(const Shape shape, const float width, const float height,
                                               const float radius)
    {
        if (!(shape == Shape::RECT || shape == Shape::CIRCLE))
        {
            LOG_ERROR("This emissionshape is not supported!");
            return *this;
        }

        if (width == 0 && height == 0 && radius == 0)
        {
            data.emShape = Shape::CAPSULE; // Denotes just the single emission point
            return *this;
        }

        data.emShape = shape;
        if (shape == Shape::RECT)
        {
            data.emp1 = width;
            data.emp2 = height;
        }
        else
        {
            data.emp1 = radius;
        }
        return *this;
    }

    //----------------- PARTICLE -----------------//

    EmitterBase& EmitterBase::setParticleShapeRect(const float width, const float height)
    {
        data.shape = Shape::RECT;
        data.p1 = width;
        data.p2 = height;
        return *this;
    }

    EmitterBase& EmitterBase::setParticleShapeCircle(const float radius)
    {
        data.shape = Shape::CIRCLE;
        data.p1 = radius;
        return *this;
    }

    EmitterBase& EmitterBase::setParticleShapeTri(Point p2, Point p3)
    {
        data.shape = Shape::TRIANGLE;
        data.p1 = p2.x;
        data.p2 = p2.y;
        data.p3 = p3.x;
        data.p4 = p3.y;
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

    EmitterBase& EmitterBase::setLifetime(const int val)
    {
        data.lifeTime = val;
        return *this;
    }

    //----------------- ADDITIONALS -----------------//

    EmitterBase& EmitterBase::setGravity(const float gravityX, const float gravityY)
    {
        data.gravX = gravityX;
        data.gravY = gravityY;
        return *this;
    }

    EmitterBase& EmitterBase::setStartScale(const float minScale, const float maxScale)
    {
        data.minScale = minScale;
        data.maxScale = maxScale;
        return *this;
    }

    EmitterBase& EmitterBase::setDirection(const float x, const float y)
    {
        data.dirX = x;
        data.dirY = y;
        return *this;
    }

    EmitterBase& EmitterBase::setSpread(const float val)
    {
        data.spreadAngle = val;
        return *this;
    }

    EmitterBase& EmitterBase::setStartVelocity(const float minVeloc, const float maxVeloc)
    {
        data.minInitVeloc = minVeloc;
        data.maxInitVeloc = maxVeloc;
        return *this;
    }

    //----------------- FUNCTIONS -----------------//


    EmitterBase& EmitterBase::setColorFunction(const ColorFunction func)
    {
        data.colorFunc = func;
        return *this;
    }

    EmitterBase& EmitterBase::setScaleFunction(const ScaleFunction func)
    {
        data.scaleFunc = func;
        return *this;
    }

    EmitterBase& EmitterBase::setResolutionScaling(const bool val)
    {
        data.resolutionScaling = val;
        return *this;
    }

    //----------------- HELPERS -----------------//

    EmitterBase::ScaleFunction EmitterBase::GetSmoothStep()
    {
        return [](const float scale, const float t) -> float { return scale * (t * t * (3 - 2 * t)); };
    }


} // namespace magique