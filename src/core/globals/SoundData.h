#ifndef SOUNDDATA_H
#define SOUNDDATA_H

#include <vector>
#include <raylib/raylib.h>

namespace magique
{

    struct Track final
    {
        static constexpr float VOLUME_STEP = 0.005F;
        static constexpr float END_THRESHOLD = 2.5F;
        Music* music = nullptr;
        float volume = 1;
        bool markedForRemoval = false;
    };


    struct SoundData final
    {
        std::vector<Sound> sounds;
        std::vector<Sound> sounds2D;
        std::vector<Playlist*> playlists;

        SoundData()
        {
            sounds.reserve(25);
            sounds2D.reserve(25);
        }


        void update() noexcept
        {
            for (auto it = currentTracks.begin(); it != currentTracks.end();)
            {
                if (it->update())
                {
                    it = currentTracks.erase(it);
                }
                else
                {
                    it++;
                }
            }
            for (auto p : playingPlaylists)
            {
                if (p->isPlaying && !IsMusicStreamPlaying(p->tracks[p->currentTrack]))
                {
                    PlayNextTrack(*p, *this);
                }
            }
        }

        void MusicStreamer::removeTrack(Music& music, bool fadeOut)
        {
            for (auto& t : currentTracks)
            {
                if (t.music == &music)
                {
                    t.markedForRemoval = true;
                    if (!fadeOut)
                    {
                        t.volume = 0;
                    }
                    return;
                }
            }
        }

        void addTrack(Music& music, bool fadeIn)
        {
            if (currentTracks.size() == maxTracks)
                return;
            for (ca& track : currentTracks)
            {
                if (track.music == &music)
                    return;
            }
            PlayMusicStream(music);
            currentTracks.emplace_back(&music, fadeIn ? 0.25F : 1.0F, false);
        }

        void MusicStreamer::startPlaylist(Playlist& playlist)
        {
            if (playlist.isPlaying || playlist.tracks.empty())
                return;
            PlayNextTrack(playlist, *this);
            playingPlaylists.push_back(&playlist);
        }

        void MusicStreamer::stopPlaylist(Playlist& playlist)
        {
            if (!playlist.isPlaying)
                return;
            for (auto it = playingPlaylists.begin(); it != playingPlaylists.end();)
            {
                if ((*it) == &playlist)
                {
                    playlist.isPlaying = false;
                    removeTrack(playlist.tracks[playlist.currentTrack]);
                    playingPlaylists.erase(it);
                    return;
                }
                ++it;
            }
        }
    };

    namespace global
    {

        inline SoundData SOUND_DATA{};

    }
} // namespace magique

#endif //SOUNDDATA_H