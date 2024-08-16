#include <magique/assets/types/Playlist.h>

namespace magique
{
    Playlist::Playlist(const std::initializer_list<Music>& musics, const bool fade, const float volume) :
        volume(volume), fading(fade)
    {
        tracks.reserve(musics.size() + 1);
        for (const auto& m : musics)
            tracks.push_back(m);
    }

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

    void Playlist::setFading(const bool val) { fading = val; }

    bool Playlist::getIsFading() const { return fading; }

    int Playlist::getSize() const { return static_cast<int>(tracks.size()); }

    bool Playlist::isPlaying() const { return isPlaying_; }

    void Playlist::setForwardFunction(const ForwardFunction func) { forwardFunction = func; }

    int Playlist::getNextTrack()
    {
        currentTrack++;
        if (forwardFunction != nullptr)
            currentTrack = forwardFunction(*this, currentTrack);
        else
        {
            currentTrack = currentTrack % static_cast<int>(tracks.size());
        }
        return currentTrack;
    }

    void Playlist::setVolume(const float newVolume) { volume = newVolume; }

} // namespace magique