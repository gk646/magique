#ifndef MAGIQUE_PARTICLEDATA_H
#define MAGIQUE_PARTICLEDATA_H

#include <vector>
#include <raylib/rlgl.h>

#include <magique/util/Logging.h>

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
            for (auto it = rectangles.begin(); it != rectangles.end();) {
                it->age++;
                if (it->age > it->lifetime) {
                    std::swap(*it, rectangles.back());
                    rectangles.pop_back();
                } else {
                    ++it;
                }
            }
            for (auto it = triangles.begin(); it != triangles.end();) {
                it->age++;
                if (it->age > it->lifetime) {
                    std::swap(*it, triangles.back());
                    triangles.pop_back();
                } else {
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