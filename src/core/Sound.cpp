// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Sound.h>
#include <magique/ecs/ECS.h>
#include <magique/util/Logging.h>

#include <magique/util/Math.h>

#include "internal/globals/AudioPlayer.h"

namespace magique
{
    void SoundSetSoundVolume(float volume)
    {
        volume = std::clamp(volume, 0.0F, 1.0F);
        auto& ap = global::AUDIO_PLAYER;
        ap.soundVolume = volume;
        for (auto& s : ap.sounds)
        {
            SetSoundVolume(s.sound, ap.getSoundVolume(s.getVolume()));
        }
    }

    float SoundGetSoundVolume() { return global::AUDIO_PLAYER.soundVolume; }

    void SoundSetMusicVolume(float volume)
    {
        volume = std::clamp(volume, 0.0F, 1.0F);
        auto& ap = global::AUDIO_PLAYER;
        ap.musicVolume = volume;
        for (auto& t : ap.tracks)
        {
            SetMusicVolume(t.music, ap.getMusicVolume(t.playBackVolume));
        }
    }

    float SoundGetMusicVolume() { return global::AUDIO_PLAYER.musicVolume; }

    void SoundPlay(const Sound& sound, const float volume, bool loop)
    {
        global::AUDIO_PLAYER.sounds.emplace_back(sound, volume, loop);
    }

    void SoundPlay2D(const Sound& sound, const entt::entity entity, const float volume, bool loop)
    {
        global::AUDIO_PLAYER.sounds.emplace_back(sound, volume, entity, loop);
    }

    void SoundPlay2D(const Sound& sound, Point pos, float volume, bool loop)
    {
        global::AUDIO_PLAYER.sounds.emplace_back(sound, volume, pos, loop);
    }

    bool SoundStop(const Sound& sound)
    {
        auto& ap = global::AUDIO_PLAYER;
        return std::erase_if(ap.sounds,
                             [&](auto& wrapper) { return wrapper.sound.stream.buffer == sound.stream.buffer; }) > 0;
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
        return false;
    }

    void SoundSetFalloffDistance(float distance) { global::AUDIO_PLAYER.maxSoundDistance = distance; }

    void MusicPlay(const Music& music, const float volume, const bool fade)
    {
        global::AUDIO_PLAYER.addTrack(music, volume, fade);
    }

    void MusicStop(const Music& music) { global::AUDIO_PLAYER.removeTrack(music); }

    void PlaylistPlay(Playlist& playlist, const float volume)
    {
        if (playlist.getSize() == 0)
        {
            LOG_ERROR("Cannot play empty playlist");
            return;
        }
        playlist.setVolume(volume);
        global::AUDIO_PLAYER.startPlaylist(playlist);
    }

    void PlaylistStop(Playlist& playlist) { global::AUDIO_PLAYER.stopPlaylist(playlist); }

    void PlaylistForward(Playlist& playlist)
    {
        global::AUDIO_PLAYER.stopPlaylist(playlist);
        global::AUDIO_PLAYER.startPlaylist(playlist);
    }

} // namespace magique
