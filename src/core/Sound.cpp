#include <magique/core/Sound.h>
#include <magique/ecs/ECS.h>
#include <magique/util/Logging.h>

#include "internal/globals/AudioPlayer.h"

namespace magique
{
    void SetSoundMasterVolume(const float volume)
    {
        auto& ap = global::AUDIO_PLAYER;
        ap.soundVolume = volume;
        for (auto& s : ap.sounds)
        {
            ::SetSoundVolume(s.sound, ap.getSoundVolume(s.volume));
        }
        for (auto& s : ap.sounds2D)
        {
            ::SetSoundVolume(s.sound, ap.getSoundVolume(s.volume));
        }
    }

    float GetSoundMasterVolume() { return global::AUDIO_PLAYER.soundVolume; }

    void SetMusicMasterVolume(const float volume)
    {
        auto& ap = global::AUDIO_PLAYER;
        ap.musicVolume = volume;
        for (auto& t : ap.tracks)
        {
            ::SetMusicVolume(t.music, ap.getMusicVolume(t.playBackVolume));
        }
    }

    float GetMusicMasterVolume() { return global::AUDIO_PLAYER.musicVolume; }

    void PlaySound(const Sound& sound, const float volume)
    {
        const auto alias = LoadSoundAlias(sound);
        SetSoundVolume(alias, global::AUDIO_PLAYER.getSoundVolume(volume));
        ::PlaySound(alias);
        global::AUDIO_PLAYER.sounds.emplace_back(alias, volume);
    }

    void PlaySound2D(const Sound& sound, const entt::entity entity, const float volume)
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

    void PlaySound2D(const Sound& sound, float& x, float& y, float volume)
    {
        const auto alias = LoadSoundAlias(sound);
        SetSoundVolume(alias, global::AUDIO_PLAYER.getSoundVolume(volume));
        ::PlaySound(alias);
        global::AUDIO_PLAYER.sounds2D.emplace_back(alias, volume, &x, &y, x, y, true);
    }

    void StopSound(const Sound& sound)
    {
        auto& ap = global::AUDIO_PLAYER;
        for (auto it = ap.sounds.begin(); it != ap.sounds.end();)
        {
            if (IsAudioStreamEqual(it->sound.stream, sound.stream))
            {
                ap.sounds.erase(it);
                return;
            }
            ++it;
        }
        for (auto it = ap.sounds2D.begin(); it != ap.sounds2D.end();)
        {
            if (IsAudioStreamEqual(it->sound.stream, sound.stream))
            {
                ap.sounds2D.erase(it);
                return;
            }
            ++it;
        }
    }

    bool IsSoundPlaying(const Sound& sound)
    {
        const auto& sd = global::AUDIO_PLAYER;
        for (const auto& s : sd.sounds)
        {
            if (IsAudioStreamEqual(s.sound.stream, sound.stream))
            {
                return true;
            }
        }
        for (const auto& s : sd.sounds2D)
        {
            if (IsAudioStreamEqual(s.sound.stream, sound.stream))
            {
                return true;
            }
        }
        return false;
    }

    void PlayMusic(const Music& music, const float volume, const bool fade)
    {
        global::AUDIO_PLAYER.addTrack(music, volume, fade);
    }

    void StopMusic(const Music& music) { global::AUDIO_PLAYER.removeTrack(music); }

    void PlayPlaylist(Playlist* playlist, const float volume)
    {
        if (playlist == nullptr)
        {
            LOG_ERROR("Passed nullptr");
            return;
        }
        if (playlist->getSize() == 0)
        {
            LOG_ERROR("Cannot play empty playlist");
            return;
        }
        playlist->setVolume(volume);
        global::AUDIO_PLAYER.startPlaylist(*playlist);
    }

    void StopPlaylist(Playlist& playlist) {
global::AUDIO_PLAYER.stopPlaylist(playlist); }

    void ForwardPlaylist(Playlist& playlist)
    {
        global::AUDIO_PLAYER.stopPlaylist(playlist);
        global::AUDIO_PLAYER.startPlaylist(playlist);
    }

} // namespace magique