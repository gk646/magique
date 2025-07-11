// SPDX-License-Identifier: zlib-acknowledgement
#include <cmath>
#include <raylib/raylib.h>

#include <entt/entity/entity.hpp>
#include <magique/core/Particles.h>
#include <magique/core/Core.h>

#include "internal/globals/ParticleData.h"

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
                particle.x = static_cast<float>(GetRandomValue(static_cast<int>(data.emX), static_cast<int>(data.emp1)));
                particle.y = static_cast<float>(GetRandomValue(static_cast<int>(data.emY), static_cast<int>(data.emp2)));
                break;
            case Shape::CIRCLE:
                {
                    const float angle = static_cast<float>(GetRandomValue(0, 360)) * (PI / 180.0f);
                    const float dist = data.emp1 * std::sqrt(static_cast<float>(GetRandomValue(0, 100)) / 100.0F);
                    particle.x = dist *std::cos(angle);
                    particle.y = dist * std::sin(angle);
                }
                break;
            case Shape::CAPSULE: // Acts as point type
                particle.x = data.emX;
                particle.y = data.emY;
                break;
            case Shape::TRIANGLE:
                MAGIQUE_ASSERT(false, "Triangle not implemented");
                break;
            }
            // Higher quality randomness should be worth it - and the random is pretty fast
            // So we call GetRandomValue() often instead of once

            // Scale
            particle.scale = data.minScale;
            if (data.minScale != data.maxScale)
            {
                const float p = static_cast<float>(GetRandomValue(0, 100)) / 100.0F;
                particle.scale = data.minScale + (data.maxScale - data.minScale) * p;
            }

            // p1,p2,p3,p4
            particle.p1 = static_cast<int16_t>(std::round(data.p1));
            particle.p2 = static_cast<int16_t>(std::round(data.p2));
            particle.p3 = static_cast<int16_t>(std::round(data.p3));
            particle.p4 = static_cast<int16_t>(std::round(data.p4));

            // Lifetime
            particle.lifeTime = data.minLife;
            if (data.minLife != data.maxLife)
            {
                const float p = static_cast<float>(GetRandomValue(0, 100)) / 100.0F;
                particle.lifeTime = static_cast<uint16_t>((float)(data.minLife + (data.maxLife - data.minLife)) * p);
            }

            // Spread
            float dirX = data.dirX;
            float dirY = data.dirY;
            if (data.spreadAngle > 0)
            {
                const float p = static_cast<float>(GetRandomValue(0, 100)) / 100.0F;
                const float spreadAngle = (-data.spreadAngle / 2.0F + data.spreadAngle * p) * DEG2RAD;
                const float currentAngle = std::atan2(data.dirY, data.dirX);
                const float newAngle = currentAngle + spreadAngle;
                dirX = std::cos(newAngle);
                dirY = std::sin(newAngle);
            }

            // vx,vy - velocity
            float velo = data.minInitVeloc;
            if (data.minInitVeloc != data.maxInitVeloc)
            {
                const float p = static_cast<float>(GetRandomValue(0, 100)) / 100.0F;
                velo = data.minInitVeloc + (data.maxInitVeloc - data.minInitVeloc) * p;
            }
            particle.vx = velo * dirX;
            particle.vy = velo * dirY;

            // Color
            if (data.colors[0] != -1) // Use pool
            {
                const int p = GetRandomValue(0, data.r - 1);
                const auto color = GetColor(data.colors[p]);
                particle.r = color.r;
                particle.g = color.g;
                particle.b = color.b;
                particle.a = color.a;
            }
            else
            {
                particle.r = data.r;
                particle.g = data.g;
                particle.b = data.b;
                particle.a = data.a;
            }

            // Rest
            particle.age = 0;
            particle.shape = data.shape;

            global::PARTICLE_DATA.addParticle(particle);
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

    EmitterBase& EmitterBase::setParticleShapeTri(const Point p2, const Point p3)
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
        data.colors[0] = -1; // Signal not using color pool
        return *this;
    }

    EmitterBase& EmitterBase::setColorPool(const std::initializer_list<Color>& colors)
    {
        if (colors.size() == 0)
        {
            LOG_ERROR("Skipping! You have to pass at least 1 color!");
            return *this;
        }

        if (colors.size() > 5)
        {
            LOG_WARNING("Passing more than 5 colors! Skipping rest");
        }

        int i = 0;
        for (const auto c : colors)
        {
            data.colors[i] = ColorToInt(c);
            ++i;
            if (i > 5)
                break;
        }
        data.r = static_cast<uint8_t>(colors.size()); // Save valid size in r

        return *this;
    }

    EmitterBase& EmitterBase::setLifetime(const int min, int max)
    {
        if (max == 0)
        {
            max = min;
        }

        if (min > max)
        {
            LOG_ERROR("Skipping! Minimum value is bigger than maximum value! Min: %d | Max: %d", min, max);
            return *this;
        }
        data.minLife = static_cast<uint16_t>(min);
        data.maxLife = static_cast<uint16_t>(max);
        return *this;
    }

    //----------------- ADDITIONALS -----------------//

    EmitterBase& EmitterBase::setGravity(const float gravityX, const float gravityY)
    {
        constexpr float TICK_CONVERSION = 1.0F / MAGIQUE_LOGIC_TICKS;
        data.gravX = gravityX * TICK_CONVERSION; // From pixel/s into pixel/tick
        data.gravY = gravityY * TICK_CONVERSION;
        return *this;
    }

    EmitterBase& EmitterBase::setScale(const float minScale, float maxScale)
    {
        if (maxScale == 0)
        {
            maxScale = minScale;
        }
        if (minScale > maxScale)
        {
            LOG_ERROR("Skipping! Minimum value is bigger than maximum value! Min: %.2f | Max: %.2f", minScale, maxScale);
            return *this;
        }
        data.minScale = minScale;
        data.maxScale = maxScale;
        return *this;
    }

    EmitterBase& EmitterBase::setDirection(const float x, const float y)
    {
        if (std::abs(x) + std::abs(y) != 1)
        {
            LOG_ERROR("Given vector isnt not a normalized direction vector! (x and y has to add up to 1)");
            return *this;
        }
        data.dirX = x;
        data.dirY = y;
        return *this;
    }

    EmitterBase& EmitterBase::setSpread(const float val)
    {
        data.spreadAngle = val;
        return *this;
    }

    EmitterBase& EmitterBase::setVelocity(const float minVeloc, float maxVeloc)
    {
        if (maxVeloc == 0)
        {
            maxVeloc = minVeloc;
        }
        if (minVeloc > maxVeloc)
        {
            LOG_ERROR("Skipping! Minimum value is bigger than maximum value! Min: %.2f | Max: %.2f", minVeloc, maxVeloc);
            return *this;
        }
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

    EmitterBase& EmitterBase::setTickFunction(const TickFunction& func)
    {
        delete (TickFunction*)data.tickFunc;
        data.tickFunc = (void*)new std::function(func);
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