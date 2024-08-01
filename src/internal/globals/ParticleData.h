#ifndef MAGIQUE_PARTICLEDATA_H
#define MAGIQUE_PARTICLEDATA_H

#include <vector>
#include <raylib/rlgl.h>

#include <magique/util/Logging.h>
#include <magique/core/Particles.h>

#include "external/raylib/src/coredata.h"

namespace magique
{
    // Data driven
    struct ParticleData final
    {
        std::vector<ScreenParticle> rectangles;
        std::vector<ScreenParticle> circles;
        std::vector<ScreenParticle> triangles;

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
            printf("Size: %d\n", rectangles.size());
            for (const auto& p : rectangles)
            {
                const Color color = {p.r, p.g, p.b, p.a};
                rlSetTexture(GetShapesTexture().id);
                Rectangle shapeRect = GetShapesTextureRectangle();
                rlNormal3f(0.0f, 0.0f, 1.0f);
                rlColor4ub(color.r, color.g, color.b, color.a);

                rlTexCoord2f(shapeRect.x / texShapes.width, shapeRect.y / texShapes.height);
                rlVertex2f(p.x, p.y);

                rlTexCoord2f(shapeRect.x / texShapes.width, (shapeRect.y + shapeRect.height) / texShapes.height);
                rlVertex2f(p.x, p.y + p.p2);

                rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width,
                             (shapeRect.y + shapeRect.height) / texShapes.height);
                rlVertex2f(p.x + p.p1, p.y + p.p2);

                rlTexCoord2f((shapeRect.x + shapeRect.width) / texShapes.width, shapeRect.y / texShapes.height);
                rlVertex2f(p.x + p.p1, p.y);

                rlEnd();
                rlSetTexture(0);
            }
        }

        void update()
        {
            for (auto it = rectangles.begin(); it != rectangles.end();)
            {
                auto& p = *it;
                const auto& emitter = p.emitter->data;
                const float relTime = static_cast<float>(p.age) / static_cast<float>(emitter.lifeTime);
                if (relTime >= 1.0F) [[unlikely]]
                {
                    std::swap(*it, rectangles.back());
                    rectangles.pop_back();
                }
                else
                {
                    ++p.age;
                    p.x += p.vx;
                    p.y += p.vy;

                    p.vx += emitter.gravX;
                    p.vy += emitter.gravY;

                    if (emitter.scaleFunc)
                    {
                        p.scale = emitter.scaleFunc(p.scale, relTime);
                    }

                    if (emitter.colorFunc)
                    {
                        p.setColor(emitter.colorFunc(p.getColor(), relTime));
                    }

                    if (emitter.tickFunc)
                    {
                        emitter.tickFunc(p, relTime);
                    }
                    ++it;
                }
            }

            for (auto it = triangles.begin(); it != triangles.end();)
            {
                it->age++;
                if (it->age > it->emitter->data.lifeTime)
                {
                    std::swap(*it, triangles.back());
                    triangles.pop_back();
                }
                else
                {
                    ++it;
                }
            }
        }
    };


    namespace global
    {

        inline ParticleData PARTICLE_DATA{};

    }
} // namespace magique

#endif //MAGIQUE_PARTICLEDATA_H