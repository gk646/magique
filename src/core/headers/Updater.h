// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_UPDATER_H
#define MAGIQUE_UPDATER_H

namespace magique::updater
{
    inline void StartTick()
    {
        PollInputEvents();
    }

    inline double EndTick(const double startTime)
    {
        const double tickTime = GetTime() - startTime;
        global::PERF_DATA.saveTickTime(UPDATE, static_cast<uint32_t>(tickTime * SEC_TO_NANOS));
        return tickTime;
    }

    inline double Tick(const double startTime, const entt::registry& reg, Game& game)
    {
        const auto& gameState = global::ENGINE_DATA.gameState;
        StartTick();
        {
            InternalUpdatePre(reg, game); // Internal update upfront
            game.updateGame(gameState);
            InternalUpdatePost();
            game.postTickUpdate(gameState);
        }
        return EndTick(startTime);
    }

} // namespace magique::updater

#endif // MAGIQUE_UPDATER_H