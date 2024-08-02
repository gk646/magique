#ifndef MAGIQUE_PARTICLEDATA_H
#define MAGIQUE_PARTICLEDATA_H

#include <vector>
#include <raylib/rlgl.h>

#include <magique/util/Logging.h>
#include <magique/core/Particles.h>
#include <magique/util/Defines.h>

namespace magique
{
    constexpr float TICK_CONVERSION = 1.0F / MAGIQUE_LOGIC_TICKS;

    // Data driven
    struct ParticleData final
    {
        std::vector<std::vector<ScreenParticle>::iterator> deleteCache;
        std::vector<ScreenParticle> rectangles;
        std::vector<ScreenParticle> circles;
        std::vector<ScreenParticle> triangles;

        ParticleData() { deleteCache.reserve(500); }

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
            rlSetTexture(GetShapesTexture().id);
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
            const auto updateVec =
                [](std::vector<ScreenParticle>& vec)
            {
                    int amount = 0;
                for (auto it = vec.begin(); it != vec.end();)
                {
                    auto& p = *it;
                    const auto& emitter = p.emitter->data;
                    const float relTime = static_cast<float>(p.age) / static_cast<float>(emitter.lifeTime);
                    if (relTime >= 1.0F) [[unlikely]]
                    {
                        auto lastEl = vec.end() - 1;
                        if (it != lastEl) {
                            *it = *lastEl;
                        }
                        vec.pop_back();
                    }
                    else
                    {
                        ++p.age;
                        p.x += p.vx;
                        p.y += p.vy;

                        p.vx += emitter.gravX * TICK_CONVERSION; // pixel/s into pixel/tick
                        p.vy += emitter.gravY * TICK_CONVERSION;

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