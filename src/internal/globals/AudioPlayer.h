// SPDX-License-Identifier: zlib-acknowledgement
#ifndef SOUNDDATA_H
#define SOUNDDATA_H

#include <raylib/raylib.h>
#include <magique/assets/types/Playlist.h>



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

        bool update();
    };

    struct SoundWrapper final
    {
        Sound sound;
        float volume;
    };

    struct Sound2D final
    {
        Sound sound;
        float volume;
        float* x;
        float* y;
        float lastX;
        float lastY;
        bool isAlive;
    };

    struct AudioPlayer final
    {
     std::vector<SoundWrapper> sounds;
     std::vector<Sound2D> sounds2D;
     std::vector<Track> tracks;
     std::vector<Playlist*> playlists;

        float soundVolume = 1.0F;
        float musicVolume = 1.0F;

        AudioPlayer()
        {
            sounds.reserve(25);
            sounds2D.reserve(25);
        }

        void addTrack(const Music& music, float volume, const bool fadeIn)
        {
            PlayMusicStream(music);
            tracks.emplace_back(music, volume, fadeIn ? 0.0F : volume, false, fadeIn);
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
            addTrack(playlist.tracks[nextTrack], playlist.volume, playlist.fading);
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
            // Check if sound is done
            for (auto it = sounds.begin(); it != sounds.end();)
            {
                if (!::IsSoundPlaying(it->sound))
                {
                    it = sounds.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            // Check if sound2D is done
            for (auto it = sounds2D.begin(); it != sounds2D.end();)
            {
                if (!::IsSoundPlaying(it->sound))
                {
                    it = sounds2D.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            // Update music buffer and remove if done
            for (auto it = tracks.begin(); it != tracks.end();)
            {
                if (it->update())
                {
                    it = tracks.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            // Progress playlist
            for (const auto playlist : playlists)
            {
                if (!IsMusicStreamPlaying(playlist->tracks[playlist->currentTrack]))
                {
                    const int nextTrack = playlist->getNextTrack();
                    addTrack(playlist->tracks[nextTrack], playlist->volume, playlist->fading);
                }
            }
        }

        [[nodiscard]] float getSoundVolume(const float playBackVolume) const
        {
            return GetMasterVolume() * soundVolume * playBackVolume;
        }

        [[nodiscard]] float getMusicVolume(const float playBackVolume) const
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
        else if (!markedForRemoval)
        {
            markedForRemoval = GetMusicTimeLength(music) - GetMusicTimePlayed(music) < FADE_DURATION;
        }

        return markedForRemoval && currentVolume <= 0;
    }
} // namespace magique

#endif //SOUNDDATA_H