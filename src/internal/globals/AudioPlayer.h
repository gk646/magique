// SPDX-License-Identifier: zlib-acknowledgement
#ifndef SOUNDDATA_H
#define SOUNDDATA_H

#include <raylib/raylib.h>
#include <magique/assets/types/Playlist.h>
#include <magique/core/Camera.h>
#include <magique/ui/UI.h>

#include "external/raylib-compat/raudio_compat.h"

namespace magique
{
    struct Track final
    {
        static constexpr float FADE_DURATION = 3.0F;
        static constexpr float VOLUME_STEP = 1.0F / (MAGIQUE_LOGIC_TICKS * FADE_DURATION); // 1 to 0 volume at x ticks/s

        Music music;
        float playBackVolume = 1.0F;
        float currentVolume = 1.0F;
        bool markedForRemoval = false;
        bool fade = true;
        bool looping = false;

        bool update();
    };

    struct SoundWrapper final
    {
        SoundWrapper(Sound sound, float volume, bool loop);
        SoundWrapper(Sound sound, float volume, Point pos, bool loop) : SoundWrapper(sound, volume, loop)
        {
            position = pos;
            isPositional = true;
        }
        SoundWrapper(Sound sound, float volume, Entity e, bool loop) : SoundWrapper(sound, volume, loop)
        {
            isPositional = true;
            entity = e;
        }

        SoundWrapper(const SoundWrapper& other) = delete;
        SoundWrapper& operator=(const SoundWrapper& other) = delete;

        SoundWrapper(SoundWrapper&& other) noexcept :
            sound(other.sound), playVolume(other.playVolume), position(other.position), entity(other.entity),
            isPositional(other.isPositional), loop(other.loop)
        {
            other.sound = {};
            other.sound.stream.buffer = nullptr;
        }

        SoundWrapper& operator=(SoundWrapper&& other) noexcept
        {
            if (this != &other)
            {
                sound = other.sound;
                playVolume = other.playVolume;
                position = other.position;
                isPositional = other.isPositional;
                entity = other.entity;
                loop = other.loop;
                isPositional = other.isPositional;

                other.sound = {};
                other.sound.stream.buffer = nullptr;
            }
            return *this;
        }

        ~SoundWrapper()
        {
            StopSound(sound);
            SetAudioBufferLooping(sound.stream.buffer, false);
            UnloadSoundAlias(sound);
            sound = {};
            sound.stream.buffer = nullptr;
        }


        Sound sound;
        float playVolume;
        Point position{};
        Entity entity = NullEntity();
        bool isPositional = false;
        bool loop = false;

        bool shouldRemove() const
        {
            if (isPositional)
                return entity != NullEntity() && !EntityExists(entity);
            return !IsSoundPlaying(sound);
        }
        float getVolume() const;
        void update();
    };

    struct AudioPlayer final
    {
        std::vector<SoundWrapper> sounds;
        std::vector<Track> tracks;
        std::vector<Playlist*> playlists;

        float maxSoundDistance = 1000;
        float soundVolume = 1.0F;
        float musicVolume = 1.0F;

        void addTrack(const Music& music, float volume, const bool fadeIn, bool looping = false)
        {
            PlayMusicStream(music);
            tracks.emplace_back(music, volume, fadeIn ? 0.0F : volume, false, fadeIn, looping);
            if (looping)
                SetAudioBufferLooping(music.stream.buffer, true);
        }

        void removeTrack(const Music& music)
        {
            for (auto& t : tracks)
            {
                if (t.music.stream.buffer == music.stream.buffer)
                {
                    t.markedForRemoval = true;
                    if (!t.fade)
                    {
                        t.currentVolume = 0;
                    }
                    return;
                }
            }
        }

        void startPlaylist(Playlist& playlist)
        {
            if (playlist.isPlaying_ || playlist.tracks.empty())
                return;

            const int nextTrack = playlist.getNextTrack();
            addTrack(playlist.tracks[nextTrack], playlist.volume, playlist.fading, false);
            playlists.push_back(&playlist);
        }

        void stopPlaylist(Playlist& playlist)
        {
            if (!playlist.isPlaying_)
                return;

            for (auto it = playlists.begin(); it != playlists.end();)
            {
                if (*it == &playlist)
                {
                    playlist.isPlaying_ = false;
                    removeTrack(playlist.tracks[playlist.currentTrack]);
                    playlists.erase(it);
                    return;
                }
                ++it;
            }
        }

        void update() noexcept
        {
            std::erase_if(sounds,
                          [](SoundWrapper& e)
                          {
                              e.update();
                              if (e.shouldRemove())
                              {
                                  return true;
                              }
                              return false;
                          });
            std::erase_if(tracks, [](Track& e) { return e.update(); });

            // Progress playlist
            for (const auto playlist : playlists)
            {
                if (!IsMusicStreamPlaying(playlist->tracks[playlist->currentTrack]))
                {
                    const int nextTrack = playlist->getNextTrack();
                    addTrack(playlist->tracks[nextTrack], playlist->volume, playlist->fading, false);
                }
            }
        }

        float getSoundVolume(const float playBackVolume) const
        {
            return GetMasterVolume() * soundVolume * playBackVolume;
        }

        float getMusicVolume(const float playBackVolume) const
        {
            return GetMasterVolume() * musicVolume * playBackVolume;
        }
    };

    namespace global
    {
        inline AudioPlayer AUDIO_PLAYER{};

    }

    inline bool Track::update()
    {
        UpdateMusicStream(music);
        auto& ap = global::AUDIO_PLAYER;
        if (markedForRemoval)
        {
            currentVolume -= VOLUME_STEP;
            SetMusicVolume(music, ap.getMusicVolume(currentVolume));
            if (currentVolume <= 0)
            {
                StopMusicStream(music);
            }
        }
        else if (currentVolume < playBackVolume)
        {
            currentVolume += VOLUME_STEP;
            SetMusicVolume(music, ap.getMusicVolume(currentVolume));
        }
        else if (!markedForRemoval && !looping)
        {
            markedForRemoval = GetMusicTimeLength(music) - GetMusicTimePlayed(music) < FADE_DURATION;
        }

        return markedForRemoval && currentVolume <= 0;
    }

    inline SoundWrapper::SoundWrapper(Sound originalSound, float volume, bool loop) :
        sound(LoadSoundAlias(originalSound)), playVolume(volume), loop(loop)
    {
        SetSoundVolume(sound, global::AUDIO_PLAYER.getSoundVolume(volume));
        PlaySound(sound);
        SetAudioBufferLooping(sound.stream.buffer, loop);
    }

    inline float SoundWrapper::getVolume() const
    {
        const auto& ap = global::AUDIO_PLAYER;
        if (isPositional)
        {
            auto dist = CameraGetPosition().euclidean(position);
            float distMult = 1.0F - MathLerpInverse(0, ap.maxSoundDistance, std::min(ap.maxSoundDistance, dist));
            return ap.getSoundVolume(playVolume * distMult);
        }
        else
        {
            return ap.getSoundVolume(playVolume);
        }
    }

    inline void SoundWrapper::update()
    {
        if (isPositional)
        {
            if (EntityExists(entity))
                position = CollisionC::GetMiddle(entity);
            auto cameraPos = CameraGetPosition();
            auto dims = UIGetTargetResolution();
            auto distFactor = (position.x - cameraPos.x) / dims.x;
            SetSoundPan(sound, distFactor);
        }
        SetSoundVolume(sound, getVolume());
    }
} // namespace magique

#endif // SOUNDDATA_H
