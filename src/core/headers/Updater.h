// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_UPDATER_H
#define MAGIQUE_UPDATER_H

namespace magique::updater
{
    inline void StartTick() {}

    inline double EndTick(const double startTime)
    {
        return global::PERF_DATA.updateTick.add(GetTime() - startTime);
    }

    inline double Tick(const double startTime, Game& game, const entt::registry& reg)
    {
        const auto& gameState = global::ENGINE_DATA.gameState;
        StartTick();
        {
            InternalUpdatePre(reg, game); // Internal update upfront
            game.onUpdateGame(gameState);
            InternalUpdatePost();
            game.onUpdateEnd(gameState);
        }
        return EndTick(startTime);
    }

} // namespace magique::updater

#endif // MAGIQUE_UPDATER_H
