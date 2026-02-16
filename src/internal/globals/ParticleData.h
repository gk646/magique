// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_PARTICLEDATA_H
#define MAGIQUE_PARTICLEDATA_H

#include <functional>
#include <raylib/rlgl.h>

#include <magique/core/Particles.h>
#include <magique/util/Logging.h>

#include "external/raylib-compat/rshapes_compat.h"
#include "magique/util/Math.h"

namespace magique
{
    // Data driven
    struct ParticleData final
    {
        std::vector<ScreenParticle> rectangles;
        std::vector<ScreenParticle> circles;
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
            }
        }

        void render() const
        {
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
                const auto& [r, g, b, a] = p.color;
                rlColor4ub(r, g, b, a);
                rlVertex2f(p.pos.x, p.pos.y);
                rlVertex2f(p.pos.x, p.pos.y + p.p2 * p.scale);
                rlVertex2f(p.pos.x + p.p1 * p.scale, p.pos.y + p.p2 * p.scale);
                rlVertex2f(p.pos.x + p.p1 * p.scale, p.pos.y);
            }

            rlEnd();
            rlSetTexture(0);
            rlDrawRenderBatchActive();
        }

        void update()
        {
            const auto updateVec = [](std::vector<ScreenParticle>& vec)
            {
                for (size_t i = 0; i < vec.size(); ++i)
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

                            p.pos.x += radialDir.x * -p.veloc.y + tangentDir.x * p.veloc.x;
                            p.pos.y += radialDir.y * -p.veloc.y + tangentDir.y * p.veloc.x;

                            p.veloc.y += p.emitter->data.angularGravity;
                        }
                        else
                        {
                            p.pos += p.veloc;
                            p.veloc+= emitter.gravity;// already changed from pixel/s into pixel/tick
                        }

                        if (emitter.scaleFunc != nullptr)
                        {
                            p.scale = emitter.scaleFunc(p.scale, relTime);
                        }

                        if (emitter.colorFunc != nullptr)
                        {
                            p.color = emitter.colorFunc(p.color, relTime);
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

#endif // MAGIQUE_PARTICLEDATA_H
