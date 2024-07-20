#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <vector>
#include <raylib/raylib.h>
#include <magique/fwd.hpp>

namespace magique
{

    struct Playlist final
    {
        // Adds a new track to the playlist
        void addTrack(const Music& music);

        // Removes a track from the playlist
        void removeTrack(const Music& music);

        // If true each track fades in and out automatically when transitioning
        void setFade(bool val);

        // Returns the number of total tracks in the playlist
        int getSize();

    private:

        std::vector<Music> tracks;
        int currentTrack = 0;
        bool isPlaying = false;
        bool fading = true;
        friend SoundData;
    };

} // namespace magique

#endif //PLAYLIST_H