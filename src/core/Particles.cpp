// SPDX-License-Identifier: zlib-acknowledgement
#include <cmath>
#include <raylib/raylib.h>

#include <entt/entity/entity.hpp>
#include <magique/core/Particles.h>
#include <magique/core/Core.h>

#include "internal/globals/ParticleData.h"
#include "magique/core/CollisionDetection.h"
#include "magique/util/Math.h"
#include "magique/util/RayUtils.h"

namespace magique
{
    void DrawParticles() { global::PARTICLE_DATA.render(); }

    void CreateScreenParticle(const ScreenEmitter& emitter, const Point& pos, const int amount)
    {
        const auto& data = emitter.data;
        data.emissionPos = pos;
        for (int i = 0; i < amount; ++i)
        {
            ScreenParticle particle;
            particle.emitter = &emitter;

            // x,y
            switch (data.emShape) //  No triangle emission shape
            {
            case Shape::RECT:
                {
                    const auto ran1 = GetRandomFloat(0.0F, 1.0F);
                    const auto ran2 = GetRandomFloat(0.0F, 1.0F);
                    if (data.volume == 1.0F)
                    {
                        particle.pos = {ran1 * data.emissionDims.x, ran2 * data.emissionDims.y};
                    }
                    else
                    {
                        const auto mult = std::sqrt(1.0F - data.volume); // Already calculated in the setter
                        // Distributed by splitting up outer rectangle in 4 rectangles
                        // Two long ones  above and bottom - and then the two sides (so they dont overlap)
                        auto rect = GetRandomValue(0, 3);
                        if (rect == 0) // top rect
                        {
                            particle.pos.x = data.emissionDims.x * ran1;
                            particle.pos.y = (data.emissionDims.y - data.emissionDims.y * mult) * 0.5F * ran2;
                        }
                        else if (rect == 1) // bottom rect
                        {
                            particle.pos.x = data.emissionDims.x * ran1;
                            particle.pos.y =
                                data.emissionDims.y - ((data.emissionDims.y - data.emissionDims.y * mult) * 0.5F * ran2);
                        }
                        else if (rect == 2) // left
                        {
                            particle.pos.x = (data.emissionDims.x - data.emissionDims.x * mult) * 0.5F * ran1;
                            particle.pos.y = data.emissionDims.y * ran2;
                        }
                        else // right
                        {
                            particle.pos.x =
                                data.emissionDims.x - (data.emissionDims.x - data.emissionDims.x * mult) * 0.5F * ran1;
                            particle.pos.y = data.emissionDims.y * ran2;
                        }
                    }
                    particle.pos += pos;
                    if (data.rotation != 0)
                    {
                        RotatePoints(data.rotation, data.anchor + pos, particle.pos, particle.pos, particle.pos,
                                     particle.pos);
                    }
                }
                break;
            case Shape::CIRCLE:
                {
                    const float angle = GetRandomFloat(0, 360) * (PI / 180.0f);
                    const float dist = data.emissionDims.x - data.emissionDims.x * data.volume * GetRandomFloat(0, 1.0F);
                    particle.pos = {pos.x + dist * std::cos(angle), pos.y + dist * std::sin(angle)};
                }
                break;
            case Shape::CAPSULE: // Acts as point type
                particle.pos = pos;
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
                const float p = GetRandomFloat(0, 1.0F);
                particle.scale = data.minScale + (data.maxScale - data.minScale) * p;
            }

            // p1,p2,p3,p4
            particle.p1 = static_cast<int16_t>(std::round(data.p1));
            particle.p2 = static_cast<int16_t>(std::round(data.p2));

            // Lifetime
            particle.lifeTime = data.minLife;
            if (data.minLife != data.maxLife)
            {
                const float p = GetRandomFloat(0, 1.0F);
                particle.lifeTime = static_cast<uint16_t>((float)(data.minLife + (data.maxLife - data.minLife)) * p);
            }

            // Spread
            Point direction = data.direction;
            if (data.spreadAngle > 0)
            {
                float angleOff = GetRandomFloat(-0.5F, 0.5F) * data.spreadAngle;
                float newAngle = GetAngleFromPoints({}, direction) + angleOff;
                direction = GetDirectionFromAngle(newAngle);
            }

            // vx,vy - velocity
            float velo = data.minInitVeloc;
            if (data.minInitVeloc != data.maxInitVeloc)
            {
                const float p = GetRandomFloat(0.0F, 1.0F);
                velo = data.minInitVeloc + (data.maxInitVeloc - data.minInitVeloc) * p;
            }
            particle.vx = velo * direction.x;
            particle.vy = velo * direction.y;

            // Color
            if (data.poolSize > 0) // Use pool
            {
                const int p = GetRandomValue(0, data.poolSize - 1);
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

    EmitterBase& EmitterBase::setEmissionShape(const Shape shape, const float width, const float height,
                                               const float radius)
    {
        if (!(shape == Shape::RECT || shape == Shape::CIRCLE))
        {
            LOG_ERROR("This emission shape is not supported!");
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
            data.emissionDims = {width, height};
        }
        else
        {
            data.emissionDims = {radius, radius};
        }
        return *this;
    }

    EmitterBase& EmitterBase::setEmissionRotation(const int angle)
    {
        data.rotation = static_cast<float>(angle % 360);
        return *this;
    }

    //----------------- PARTICLE -----------------//

    EmitterBase& EmitterBase::setEmissionRotationAnchor(const Point& anchor)
    {
        data.anchor = anchor;
        return *this;
    }

    EmitterBase& EmitterBase::setEmissionShapeVolume(float percent)
    {
        if (percent < 0 || percent > 1)
        {
            LOG_ERROR("Invalid percent value!");
            return *this;
        }
        if (percent == 0)
            percent = 0.001F;
        data.volume = percent;
        return *this;
    }

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

    EmitterBase& EmitterBase::setColor(const Color& color)
    {
        data.r = color.r;
        data.g = color.g;
        data.b = color.b;
        data.a = color.a;
        data.poolSize = 0; // Signal not using color pool
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
            if (i >= 5)
                break;
            data.colors[i] = ColorToInt(c);
            ++i;
        }
        data.poolSize = static_cast<uint8_t>(colors.size());
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

    EmitterBase& EmitterBase::setDirection(const Point& direction)
    {
        if (direction != 0 && std::abs(direction.x) + std::abs(direction.y) > 1.43F)
        {
            LOG_ERROR("Given vector isnt not a normalized direction vector!");
            return *this;
        }
        data.direction = direction;
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

    const internal::EmitterData& EmitterBase::getData() const { return data; }

} // namespace magique