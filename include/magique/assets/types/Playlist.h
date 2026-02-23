// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_PLAYLIST_H
#define MAGIQUE_PLAYLIST_H

#include <vector>
#include <raylib/raylib.h>
#include <magique/fwd.hpp>

//===============================================
// Playlist
//===============================================
// .....................................................................
// Abstracts a playlist with automatic fading and optional custom forward function
// .....................................................................

namespace magique
{
    struct Playlist final
    {
        using ForwardFunction = int (*)(const Playlist& self, int currTrack);

        Playlist(const std::initializer_list<Music>& musics, bool fade = true, float volume = 1.0F);

        // Adds a new track to the playlist
        void addTrack(const Music& music);

        // Removes a track from the playlist
        void removeTrack(const Music& music);

        // If true each track fades in and out automatically when transitioning
        // Default: True
        void setFading(bool val);
        bool getIsFading() const;

        // Returns the number of total tracks in the playlist
        [[nodiscard]] int getSize() const;

        // Returns true if the playlist is currently being played
        [[nodiscard]] bool isPlaying() const;

        // If set this determines the next track - called everytime a track ends to determine the next
        void setForwardFunction(ForwardFunction func);

        // Sets the volume of the playlist tracks - change is active with the next track from the playlist
        void setVolume(float volume);

    private:
        [[nodiscard]] int getNextTrack();

        std::vector<Music> tracks;
        ForwardFunction forwardFunction = nullptr;
        int currentTrack = 0;
        float volume = 1.0F;
        bool isPlaying_ = false;
        bool fading = true;
        friend AudioPlayer;
    };

} // namespace magique

#endif //MAGIQUE_PLAYLIST_H