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
        for (const auto& t : ap.tracks)
        {
            SetMusicVolume(t.music, ap.getMusicVolume(t.playBackVolume));
        }
    }

    float SoundGetMusicVolume() { return global::AUDIO_PLAYER.musicVolume; }

    void SoundPlay(const Sound& sound, const float volume, bool loop)
    {
        if (IsSoundValid(sound)) [[likely]]
            global::AUDIO_PLAYER.sounds.emplace_back(sound, volume, loop);
        else
            LOG_WARNING("Cant play invalid sound");
    }

    void SoundPlay2D(const Sound& sound, const Entity entity, const float volume, bool loop)
    {
        if (IsSoundValid(sound)) [[likely]]
            global::AUDIO_PLAYER.sounds.emplace_back(sound, volume, entity, loop);
        else
            LOG_WARNING("Cant play invalid sound");
    }

    void SoundPlay2D(const Sound& sound, Point pos, float volume, bool loop)
    {
        if (IsSoundValid(sound)) [[likely]]
            global::AUDIO_PLAYER.sounds.emplace_back(sound, volume, pos, loop);
        else
            LOG_WARNING("Cant play invalid sound");
    }

    bool SoundStop(const Sound& sound)
    {
        auto& ap = global::AUDIO_PLAYER;
        return std::erase_if(ap.sounds,
                             [&](auto& wrapper) { return wrapper.sound.stream.buffer == sound.stream.buffer; }) > 0;
    }

    bool SoundStop(Entity entity)
    {
        auto& ap = global::AUDIO_PLAYER;
        return std::erase_if(ap.sounds, [&](const SoundWrapper& wrapper) { return wrapper.entity == entity; }) > 0;
    }

    bool SoundIsPlaying(const Sound& sound)
    {
        auto& ap = global::AUDIO_PLAYER;
        return std::ranges::any_of(ap.sounds, [&](const SoundWrapper& wrapper)
                                   { return wrapper.sound.stream.buffer == sound.stream.buffer; });
    }

    bool SoundIsPlaying(Entity entity)
    {
        auto& ap = global::AUDIO_PLAYER;
        return std::ranges::any_of(ap.sounds, [&](const SoundWrapper& wrapper) { return wrapper.entity == entity; });
    }

    void SoundSetFalloffDistance(float distance) { global::AUDIO_PLAYER.maxSoundDistance = distance; }

    void MusicPlay(const Music& music, const float volume, const bool fade, bool loop)
    {
        global::AUDIO_PLAYER.addTrack(music, volume, fade, loop);
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
