// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Sound.h>
#include <magique/ecs/ECS.h>
#include <magique/util/Logging.h>
#include <magique/internal/Macros.h>
#include <magique/util/Math.h>

#include "internal/globals/AudioPlayer.h"

namespace magique
{
    void SoundSetMasterVolume(float volume)
    {
        volume = clamp(volume, 0.0F, 1.0F);
        auto& ap = global::AUDIO_PLAYER;
        ap.soundVolume = volume;
        for (auto& s : ap.sounds)
        {
            SetSoundVolume(s.sound, ap.getSoundVolume(s.volume));
        }
        for (auto& s : ap.sounds2D)
        {
            SetSoundVolume(s.sound, ap.getSoundVolume(s.volume));
        }
    }

    float SoundGetMasterVolume() { return global::AUDIO_PLAYER.soundVolume; }

    void SoundSetMusicVolume(float volume)
    {
        volume = clamp(volume, 0.0F, 1.0F);
        auto& ap = global::AUDIO_PLAYER;
        ap.musicVolume = volume;
        for (auto& t : ap.tracks)
        {
            SetMusicVolume(t.music, ap.getMusicVolume(t.playBackVolume));
        }
    }

    float SoundGetMusicVolume() { return global::AUDIO_PLAYER.musicVolume; }

    void SoundPlay(const Sound& sound, const float volume)
    {
        const auto alias = LoadSoundAlias(sound);
        SetSoundVolume(alias, global::AUDIO_PLAYER.getSoundVolume(volume));
        ::PlaySound(alias);
        global::AUDIO_PLAYER.sounds.emplace_back(alias, volume);
    }

    void SoundPlay2D(const Sound& sound, const entt::entity entity, const float volume)
    {
        auto& reg = GetRegistry();
        if (!reg.valid(entity))
        {
            LOG_ERROR("Passed invalid entity: %d", static_cast<int>(entity));
            return;
        }
        const auto alias = LoadSoundAlias(sound);
        SetSoundVolume(alias, global::AUDIO_PLAYER.getSoundVolume(volume));
        ::PlaySound(alias);
        auto& pos = GetRegistry().get<PositionC>(entity);
        global::AUDIO_PLAYER.sounds2D.emplace_back(alias, volume, &pos.x, &pos.y, pos.x, pos.y, true);
    }

    void SoundPlay2D(const Sound& sound, float& x, float& y, float volume)
    {
        const auto alias = LoadSoundAlias(sound);
        SetSoundVolume(alias, global::AUDIO_PLAYER.getSoundVolume(volume));
        ::PlaySound(alias);
        global::AUDIO_PLAYER.sounds2D.emplace_back(alias, volume, &x, &y, x, y, true);
    }

    void SoundStop(const Sound& sound)
    {
        auto& ap = global::AUDIO_PLAYER;
        for (auto it = ap.sounds.begin(); it != ap.sounds.end();)
        {
            if (it->sound.stream.buffer == sound.stream.buffer)
            {
                ap.sounds.erase(it);
                return;
            }
            ++it;
        }
        for (auto it = ap.sounds2D.begin(); it != ap.sounds2D.end();)
        {
            if (it->sound.stream.buffer == sound.stream.buffer)
            {
                ap.sounds2D.erase(it);
                return;
            }
            ++it;
        }
    }

    bool SoundIsPlaying(const Sound& sound)
    {
        const auto& sd = global::AUDIO_PLAYER;
        for (const auto& s : sd.sounds)
        {
            if (s.sound.stream.buffer == sound.stream.buffer)
            {
                return true;
            }
        }
        for (const auto& s : sd.sounds2D)
        {
            if (s.sound.stream.buffer == sound.stream.buffer)
            {
                return true;
            }
        }
        return false;
    }

    void SoundPlayMusic(const Music& music, const float volume, const bool fade)
    {
        global::AUDIO_PLAYER.addTrack(music, volume, fade);
    }

    void SoundStopMusic(const Music& music) { global::AUDIO_PLAYER.removeTrack(music); }

    void SoundPlayPlaylist(Playlist& playlist, const float volume)
    {
        if (playlist.getSize() == 0)
        {
            LOG_ERROR("Cannot play empty playlist");
            return;
        }
        playlist.setVolume(volume);
        global::AUDIO_PLAYER.startPlaylist(playlist);
    }

    void SoundStopPlaylist(Playlist& playlist) { global::AUDIO_PLAYER.stopPlaylist(playlist); }

    void SoundForwardPlaylist(Playlist& playlist)
    {
        global::AUDIO_PLAYER.stopPlaylist(playlist);
        global::AUDIO_PLAYER.startPlaylist(playlist);
    }

} // namespace magique
