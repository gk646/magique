// SPDX-License-Identifier: zlib-acknowledgement
#include <cmath>
#include <raylib/raylib.h>
#include <algorithm>

#include <entt/entity/entity.hpp>
#include <magique/core/Particles.h>

#include "enchantum/enchantum.hpp"

#include <magique/core/Engine.h>
#include <magique/util/Math.h>
#include <magique/util/RayUtils.h>
#include <magique/core/Collision.h>

#include "internal/globals/ParticleData.h"

namespace magique
{
    void ParticlesDraw(ParticleLayer layer) { global::PARTICLE_DATA.render(layer); }

    float ParticlesGetAmountScale() { return global::PARTICLE_DATA.scale; }

    void ParticlesSetAmountScale(float amount) { global::PARTICLE_DATA.scale = amount; }

    void ParticlesEmit(const ScreenEmitter& emitter, Point pos, int amount, ParticleLayer layer)
    {
        const auto& data = emitter.data;
        data.emissionPos = pos;
        const int finalAmount = static_cast<int>((float)amount * global::PARTICLE_DATA.scale);
        for (int i = 0; i < finalAmount; ++i)
        {
            ScreenParticle particle;
            particle.emitter = &emitter;
            particle.layer = layer;
            particle.emissionRotation = emitter.data.rotation;

            // x,y
            switch (data.emShape) //  No triangle emission shape
            {
            case Shape::RECT:
                {
                    const auto ran1 = MathRandom(0.0F, 1.0F);
                    const auto ran2 = MathRandom(0.0F, 1.0F);
                    if (data.volume == 1.0F)
                    {
                        particle.pos = Point{ran1 * (data.emissionDims.x), ran2 * (data.emissionDims.y)};
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
                    particle.emissionCenter = pos + data.emissionDims / 2;
                    if (data.rotation != 0)
                    {
                        RotatePoint(data.rotation, pos + data.emissionAnchor, particle.pos);
                        RotatePoint(data.rotation, pos + data.emissionAnchor, particle.emissionCenter);
                    }
                }
                break;
            case Shape::CIRCLE:
                {
                    const float angle = MathRandom(0, 360) * (PI / 180.0f);
                    const float dist = data.emissionDims.x - data.emissionDims.x * data.volume * MathRandom(0, 1.0F);
                    particle.pos = {pos.x + dist * std::cos(angle), pos.y + dist * std::sin(angle)};
                    particle.emissionCenter = pos;
                }
                break;
            case Shape::TRIANGLE: // Acts as point type
                particle.pos = pos;
                if (data.rotation != 0)
                {
                    RotatePoint(data.rotation, pos + data.emissionAnchor, particle.pos);
                    RotatePoint(data.rotation, pos + data.emissionAnchor, particle.emissionCenter);
                }
                break;
            }

            if (emitter.data.shape == Shape::CIRCLE)
            {
                particle.pos += emitter.data.particleDims / 2;
            }
            particle.pos.floor();

            // Higher quality randomness should be worth it - and the random is pretty fast
            // So we call GetRandomValue() often instead of once

            // Scale
            particle.scale = data.minScale;
            if (data.minScale != data.maxScale)
            {
                const float p = MathRandom(0, 1.0F);
                particle.scale = data.minScale + (data.maxScale - data.minScale) * p;
            }

            // p1,p2,p3,p4
            particle.p1 = static_cast<int16_t>(std::round(data.particleDims.x));
            particle.p2 = static_cast<int16_t>(std::round(data.particleDims.y));

            // Lifetime
            float lifeSecs = MathRandom(0, 1.0F) * (data.lifeTime.y - data.lifeTime.x) + data.lifeTime.x;
            particle.lifeTime = (int)std::round(lifeSecs * (float)MAGIQUE_LOGIC_TICKS);

            // Spread
            Point direction = data.direction;
            if (data.spreadAngle > 0)
            {
                float angleOff = MathRandom(-0.5F, 0.5F) * data.spreadAngle;
                direction = Point::FromRotation(direction.rotation() + angleOff);
            }

            // vx,vy - velocity
            float velo = data.minInitVeloc;
            if (data.minInitVeloc != data.maxInitVeloc)
            {
                const float p = MathRandom(0.0F, 1.0F);
                velo = data.minInitVeloc + (data.maxInitVeloc - data.minInitVeloc) * p;
            }
            particle.veloc = direction * velo;

            Color best = data.colors.front().color;
            float maxWeight = 0.0F;
            for (auto& wColor : data.colors)
            {
                auto weight = MathRandom() * wColor.weight ;
                if (weight > maxWeight)
                {
                    maxWeight = weight;
                    best = wColor.color;
                }
            }
            particle.color = best;

            // Rest
            particle.age = 0;
            particle.shape = data.shape;

            if (data.angularVelocity.x != 0 || data.angularVelocity.y != 0 || data.angularGravity != 0)
            {
                particle.angular = true;
                particle.veloc = data.angularVelocity * velo;
            }

            global::PARTICLE_DATA.addParticle(particle);
        }
    }

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
            data.emShape = Shape::TRIANGLE; // Denotes just the single emission point
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

    EmitterBase& EmitterBase::setEmissionRotation(const float angle)
    {
        data.rotation = std::clamp(angle, 0.0F, 360.0F);
        return *this;
    }

    //----------------- PARTICLE -----------------//

    EmitterBase& EmitterBase::setEmissionRotationAnchor(const Point& anchor)
    {
        data.emissionAnchor = anchor.floor();
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

    EmitterBase& EmitterBase::setParticleShapeRect(Point dims)
    {
        data.shape = Shape::RECT;
        data.particleDims = dims;
        return *this;
    }

    EmitterBase& EmitterBase::setParticleShapeCircle(const float radius)
    {
        data.shape = Shape::CIRCLE;
        data.particleDims = radius;
        return *this;
    }

    EmitterBase& EmitterBase::setColors(const std::vector<Color>& colors)
    {
        if (colors.empty())
        {
            LOG_ERROR("Skipping! You have to pass at least 1 color!");
            return *this;
        }
        data.colors.clear();
        for (auto& color : colors)
        {
            data.colors.emplace_back(color, 0.5F);
        }
        return *this;
    }

    EmitterBase& EmitterBase::setColorsWeighted(const std::vector<WeightedColor>& colors)
    {
        if (colors.empty())
        {
            LOG_ERROR("Skipping! You have to pass at least 1 color!");
            return *this;
        }
        data.colors = colors;
        return *this;
    }

    EmitterBase& EmitterBase::setLifetime(Point lifetime)
    {
        if (lifetime.y == 0)
        {
            lifetime.x = 0;
        }

        if (lifetime.x > lifetime.y)
        {
            LOG_ERROR("Skipping! Minimum value is bigger than maximum value! Min: %.2f | Max: %.2f", lifetime.x,
                      lifetime.y);
            return *this;
        }
        data.lifeTime = lifetime;
        return *this;
    }

    EmitterBase& EmitterBase::setAngularGravity(float gravity)
    {
        data.angularGravity = gravity;
        return *this;
    }

    EmitterBase& EmitterBase::setAngularVelocity(const Point& velocity)
    {
        data.angularVelocity = velocity;
        return *this;
    }

    //----------------- ADDITIONALS -----------------//

    EmitterBase& EmitterBase::setGravity(Point gravity)
    {
        constexpr float TICK_CONVERSION = 1.0F / MAGIQUE_LOGIC_TICKS;
        data.gravity = gravity * TICK_CONVERSION; // From pixel/s into pixel/tick
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

    EmitterBase& EmitterBase::setVelocityRange(const float minVeloc, float maxVeloc)
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

    //----------------- HELPERS -----------------//

    EmitterBase::ScaleFunction EmitterBase::GetSmoothStep()
    {
        return [](const float scale, const float t) -> float
        {
            return scale * (t * t * (3 - 2 * t));
        };
    }

    const internal::EmitterData& EmitterBase::getData() const { return data; }

} // namespace magique
