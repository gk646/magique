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
            std::erase_if(tweens,
                          [](Tween* tween)
                          {
                              tween->update();

                              if (tween->tickFunc)
                              {
                                  tween->tickFunc(*tween);
                              }

                              if (tween->isDone())
                              {
                                  tween->started = false;
                                  return true;
                              }
                              return false;
                          });
        }

        void add(Tween& tween)
        {
            if (!std::ranges::contains(tweens, &tween))
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
