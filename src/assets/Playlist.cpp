#include <magique/assets/types/Playlist.h>

namespace magique
{
    void Playlist::addTrack(const Music& music) { tracks.push_back(music); }

    void Playlist::removeTrack(const Music& music)
    {
        for (auto it = tracks.begin(); it != tracks.end();)
        {
            if (IsAudioStreamEqual(music.stream, it->stream))
            {
                it = tracks.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void Playlist::setFade(const bool val) { fading = val; }

    int Playlist::getSize() const { return static_cast<int>(tracks.size()); }

    bool Playlist::isPlaying() const { return isPlaying_; }

    void Playlist::setForwardFunction(const ForwardFunction func) { forwardFunction = func; }

    int Playlist::getNextTrack()
    {
        int nextTrack = 0;
        if (forwardFunction)
            nextTrack = forwardFunction(*this, currentTrack);
        else
        {
            nextTrack = currentTrack % static_cast<int>(tracks.size());
        }
        currentTrack++;
        return nextTrack;
    }

} // namespace magique