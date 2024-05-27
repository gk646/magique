#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <raylib.h>

#include "MusicStreamer.h"
#include "SpatialPlayer.h"

struct SoundHandler {
    fast_vector<Sound> currentSounds;
    MusicStreamer streamer;
    SpatialPlayer spatial;

    void update();
    //-----------SOUND-----------// //.wav
    void playSound(const Sound& s);
    void playSpatialSound(const Sound& s, float& attachX, float& attachY);

    //-----------MUSIC-----------// //Streamed .mp3
    void playMusic(Music& m);
    void stopMusic(Music& m);
    bool isMusicPlaying(const Music& m);

    //-----------PLAYLIST-----------// //Streamed aggregated .mp3
    void playPlaylist(Playlist& p);
    void stopPlaylist(Playlist& p);
    bool isPlaylistPlaying(const Playlist& p);
};
#endif //AUDIOPLAYER_H