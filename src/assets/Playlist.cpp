#include <magique/assets/types/Playlist.h>


namespace magique
{

    void Playlist::addTrack(const Music& music)
    {
        tracks.push_back(music);
    }

    void Playlist::removeTrack(const Music& music)
    {
       for(auto it = tracks.begin(); it != tracks.end())
       {
           if(music.stream.buffer.)
       }
    }


}