#ifndef TWEENDATA_H
#define TWEENDATA_H

#include <magique/gamedev/Tweens.h>
#include "internal/datastructures/VectorType.h"

namespace magique
{

    struct TweenData final
    {
        vector<Tween*> tweens;

        void update()
        {
            for (int i = 0; i < tweens.size(); ++i)
            {
                auto& tween = tweens[i];
                tween->step = std::min(1.0F, tween->step + tween->stepWidth);
                if (tween->step >= 1.0F)
                {
                    tween->started = false;
                    if (tween->callback)
                        tween->callback();
                    tween->started = false;
                    tweens.erase(&tween);
                    i--;
                }
            }
        }

        void add(Tween& tween)
        {
            if (tweens.contains(&tween))
                return;

            tweens.push_back(&tween);
        }

        void remove(Tween& tween) { tweens.erase(&tween); }
    };

    namespace global
    {
        inline TweenData TWEEN_DATA{};
    }
} // namespace magique

#endif //TWEENDATA_H