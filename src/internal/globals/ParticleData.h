#ifndef MAGIQUE_PARTICLEDATA_H
#define MAGIQUE_PARTICLEDATA_H

#include <functional>
#include <raylib/rlgl.h>

#include <magique/core/Particles.h>
#include <magique/util/Logging.h>

#include "external/raylib-compat/rshapes_compat.h"
#include "internal/datastructures/VectorType.h"

namespace magique
{
    // Data driven
    struct ParticleData final
    {
        vector<ScreenParticle> rectangles;
        vector<ScreenParticle> circles;
        vector<ScreenParticle> triangles;

        void addParticle(const ScreenParticle& sp)
        {
            switch (sp.shape)
            {
            case Shape::RECT:
                rectangles.push_back(sp);
                break;
            case Shape::CIRCLE:
                circles.push_back(sp);
                break;
            case Shape::TRIANGLE:
                triangles.push_back(sp);
                break;
            case Shape::CAPSULE:
                LOG_ERROR("Shape not supported");
                break;
            }
        }

        void render() const
        {
            // TODO optimize with custom vertex buffer - could even be filled multithreaded
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
                rlVertex2f(p.x, p.y);
                rlVertex2f(p.x, p.y + p.p2 * p.scale);
                rlVertex2f(p.x + p.p1 * p.scale, p.y + p.p2 * p.scale);
                rlVertex2f(p.x + p.p1 * p.scale, p.y);
            }

            for (const auto& p : triangles)
            {
                rlColor4ub(p.r, p.g, p.b, p.a);
                rlVertex2f(p.x, p.y);
                rlVertex2f(p.x + p.p1 * p.scale, p.y + p.p2 * p.scale);
                rlVertex2f(p.x + p.p3 * p.scale, p.y + p.p4 * p.scale);
            }

            for (const auto& p : circles)
            {
                LOG_FATAL("Method not implemented");
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
                        p.x += p.vx;
                        p.y += p.vy;

                        p.vx += emitter.gravX; // already changed from pixel/s into pixel/tick
                        p.vy += emitter.gravY;

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
            updateVec(triangles);
            updateVec(circles);
        }
    };

    namespace global
    {

        inline ParticleData PARTICLE_DATA{};

    }
} // namespace magique

#endif //MAGIQUE_PARTICLEDATA_H