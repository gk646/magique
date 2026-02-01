#ifndef TWEENDATA_H
#define TWEENDATA_H

#include <algorithm>

#include <magique/gamedev/Tweens.h>

namespace magique
{

    struct TweenData final
    {
        std::vector<Tween*> tweens;

        void update()
        {
            for (auto it = tweens.begin(); it != tweens.end();)
            {
                auto& tween = **it;
                tween.step = std::min(1.0F, tween.step + tween.stepWidth);

                if (tween.tickFunc)
                {
                    tween.tickFunc(tween);
                }

                if (tween.isDone())
                {
                    tween.started = false;
                    it = tweens.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

        void add(Tween& tween)
        {
            if (std::ranges::contains(tweens, &tween))
            {
                return;
            }
            tweens.push_back(&tween);
        }

        void remove(Tween& tween) { std::erase(tweens, &tween); }
    };

    namespace global
    {
        inline TweenData TWEEN_DATA{};
    }
} // namespace magique

#endif // TWEENDATA_H
