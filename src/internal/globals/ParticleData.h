// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_PARTICLEDATA_H
#define MAGIQUE_PARTICLEDATA_H

#include <functional>
#include <raylib/rlgl.h>

#include <magique/core/Particles.h>
#include <magique/util/Logging.h>

#include "external/raylib-compat/rshapes_compat.h"
#include "internal/datastructures/VectorType.h"
#include "magique/util/Math.h"

namespace magique
{
    // Data driven
    struct ParticleData final
    {
        vector<ScreenParticle> rectangles;
        vector<ScreenParticle> circles;
        float scale = 1.0F;

        void addParticle(const ScreenParticle& sp)
        {
            switch (sp.shape)
            {
            case Shape::RECT:
                rectangles.push_back(sp);
                break;
            case Shape::CIRCLE:
                LOG_FATAL("Method not implemented");
                break;
            case Shape::TRIANGLE:
                LOG_ERROR("Shape not supported");
                break;
            case Shape::CAPSULE:
                LOG_ERROR("Shape not supported");
                break;
            }
        }

        void render() const
        {
            // TODO optimize with custom vertex buffer - could even be filled multithreaded
            // Preallocate it - like with the hard shadows

            rlSetTexture(GetShapesTexture().id);
            const auto shapeRect = GetTexShapesRect();
            const auto& texShapes = GetTexShapes();

            rlTexCoord2f(shapeRect.x / texShapes.width, shapeRect.y / texShapes.height);
            rlTexCoord2f(shapeRect.x / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
            rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width,
                         (shapeRect.y + shapeRect.height) / texShapes.height);
            rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, shapeRect.y / texShapes.height);

            rlBegin(RL_QUADS);
            for (const auto& p : rectangles)
            {
                rlColor4ub(p.r, p.g, p.b, p.a);
                rlVertex2f(p.pos.x, p.pos.y);
                rlVertex2f(p.pos.x, p.pos.y + p.p2 * p.scale);
                rlVertex2f(p.pos.x + p.p1 * p.scale, p.pos.y + p.p2 * p.scale);
                rlVertex2f(p.pos.x + p.p1 * p.scale, p.pos.y);
            }

            rlEnd();
            rlSetTexture(0);
        }

        void update()
        {
            const auto updateVec = [](vector<ScreenParticle>& vec)
            {
                for (int i = 0; i < vec.size(); ++i)
                {
                    auto& p = vec[i];
                    const float relTime = static_cast<float>(p.age) / static_cast<float>(p.lifeTime);
                    if (relTime >= 1.0F) [[unlikely]]
                    {
                        vec[i] = vec.back();
                        vec.pop_back();
                    }
                    else
                    {
                        const auto& emitter = p.emitter->data;
                        ++p.age;

                        if (p.angular) [[unlikely]]
                        {
                            auto diff = Point{p.pos.x, p.pos.y} - p.emissionCenter;
                            const float radius = diff.magnitude();

                            auto radialDir = diff / radius;
                            const auto tangentDir = radialDir.perpendicular(true);

                            p.pos.x += radialDir.x * -p.vy + tangentDir.x * p.vx;
                            p.pos.y += radialDir.y * -p.vy + tangentDir.y * p.vx;

                            p.vy += p.emitter->data.angularGravity;
                        }
                        else
                        {
                            p.pos.x += p.vx;
                            p.pos.y += p.vy;
                            p.vx += emitter.gravX; // already changed from pixel/s into pixel/tick
                            p.vy += emitter.gravY;
                        }

                        if (emitter.scaleFunc != nullptr)
                        {
                            p.scale = emitter.scaleFunc(p.scale, relTime);
                        }

                        if (emitter.colorFunc != nullptr)
                        {
                            p.setColor(emitter.colorFunc(p.getColor(), relTime));
                        }

                        if (emitter.tickFunc != nullptr)
                        {
                            (*static_cast<EmitterBase::TickFunction*>(emitter.tickFunc))(p, relTime);
                        }
                    }
                }
            };
            updateVec(rectangles);
            updateVec(circles);
        }
    };

    namespace global
    {

        inline ParticleData PARTICLE_DATA{};

    }
} // namespace magique

#endif //MAGIQUE_PARTICLEDATA_H