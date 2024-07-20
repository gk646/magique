#include <magique/core/Sound.h>

#include "external/raylib/src/rcore.c"
#include "core/globals/SoundData.h"

namespace magique
{
    void PlaySound(const Sound& sound, const float volume)
    {
        const auto alias = LoadSoundAlias(sound);
        SetSoundVolume(alias, volume);
        global::SOUND_DATA.sounds.push_back(alias);
        ::PlaySound(sound);
    }

    bool IsSoundPlaying(const Sound& sound)
    {
        const auto& sd = global::SOUND_DATA;
        for (const auto& s : sd.sounds)
        {
            if (IsSoundAlias(s, sound) && ::IsSoundPlaying(s))
            {
                return true;
            }
        }

        for (const auto& s : sd.sounds2D)
        {
            if (IsSoundAlias(s, sound) && ::IsSoundPlaying(s))
            {
                return true;
            }
        }
        return false;
    }

    void PlayMusic(const Music& music)
    {
PlayMusic(music);
    }
} // namespace magique