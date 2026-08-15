// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_SOUND_DATA_H
#define MAGIQUE_SOUND_DATA_H

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
        Sound sound;
        float playVolume;
        float originalPitch;
        Point position{};
        Entity entity = NullEntity{};
        bool isPositional = false;
        bool loop = false;

        SoundWrapper(Sound sound, float volume, float pitch, bool loop, bool isPositional = false, Point position = {},
                     Entity entity = NullEntity{});

        SoundWrapper(const SoundWrapper& other) = delete;
        SoundWrapper& operator=(const SoundWrapper& other) = delete;

        SoundWrapper(SoundWrapper&& other) noexcept :
            sound(other.sound), playVolume(other.playVolume), originalPitch(other.originalPitch),
            position(other.position), entity(other.entity), isPositional(other.isPositional), loop(other.loop)
        {
            other.sound = {};
        }

        SoundWrapper& operator=(SoundWrapper&& other) noexcept
        {
            if (this != &other)
            {
                StopSound(sound);
                SetAudioBufferLooping(sound.stream.buffer, false);
                sound = other.sound;
                playVolume = other.playVolume;
                position = other.position;
                entity = other.entity;
                isPositional = other.isPositional;
                loop = other.loop;
                SetAudioBufferLooping(sound.stream.buffer, loop);

                other.sound = {};
            }
            return *this;
        }

        ~SoundWrapper()
        {
            StopSound(sound);
            UnloadSoundAlias(sound);
            sound = {};
        }


        bool shouldRemove() const
        {
            if (isPositional)
            {
                if (entity == NullEntity{})
                    return !IsSoundPlaying(sound);
                return !EntityExists(entity);
            }
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
        Point soundPitchInterval = 1;

        void addTrack(const Music& music, float volume, const bool fadeIn, bool looping = false)
        {
            PlayMusicStream(music);
            tracks.emplace_back(music, volume, fadeIn ? 0.0F : volume, false, fadeIn, looping);
            SetAudioBufferLooping(music.stream.buffer, looping);
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
                              return e.shouldRemove();
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

    inline SoundWrapper::SoundWrapper(Sound original, float volume, float pitch, bool loop, bool isPositional,
                                      Point position, Entity entity) :
        sound(LoadSoundAlias(original)), playVolume(volume), originalPitch(pitch), position(position), entity(entity),
        isPositional(isPositional), loop(loop)
    {
        SetAudioBufferLooping(sound.stream.buffer, loop);
        update();
        auto [min, max] = global::AUDIO_PLAYER.soundPitchInterval * pitch;
        SetSoundPitch(sound, MathRandom(min, max));
        PlaySound(sound);
    }

    inline float SoundWrapper::getVolume() const
    {
        const auto& ap = global::AUDIO_PLAYER;
        if (isPositional)
        {
            auto dist = CameraGetPosition().euclidean(position);
            float distMult = 1.0F - MathLerpInverse(0, ap.maxSoundDistance, std::min(ap.maxSoundDistance, dist));
            distMult = std::clamp(distMult, 0.0F, 1.0F);
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

#endif // MAGIQUE_SOUND_DATA_H
