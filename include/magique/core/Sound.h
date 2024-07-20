#ifndef MAGIQUE_AUDIO_H
#define MAGIQUE_AUDIO_H

#include <magique/fwd.hpp>
#include <entt/entity/fwd.hpp>
#include <magique/assets/types/Playlist.h>

//-----------------------------------------------
// Sound Module
//-----------------------------------------------
// .....................................................................
// Supported Sound and Music formats: mp3, wav, flac, ogg, qoa (all raylib formats)
// Note: Sound refers to shorter and music to longer tracks - music is streamed and is recommended for anything above 10s!
// Note: The volume for each track is relative to the global volume
// .....................................................................

namespace magique
{
    //----------------- SOUND & SOUND2D -----------------//

    // Plays the given sound - multiple calls play it multiple times
    void PlaySound(const Sound& sound, float volume = 1.0F);

    // Plays the given sound attached to the given entity
    // Automatically handles lifetime if entities is destroyed
    void PlaySound2D(const Sound& sound, entt::entity entity, float volume = 1.0F);

    // Plays the given sound attached to the given coordinates
    // IMPORTANT: Given references MUST remain valid for the full duration of the sound
    void PlaySound2D(const Sound& sound, float& x, float& y, float volume = 1.0F);

    // Returns true if any instance of the given sound is currently playing
    // Checks both normal and 2D
    bool IsSoundPlaying(const Sound& sound);

    //----------------- MUSIC -----------------//

    // Plays the given music
    void PlayMusic(const Music& music, float volume = 1.0F);

    //----------------- PLAYLIST -----------------//

    void PlayPlaylist(const Playlist& playlist);

    void StopPlaylist(const Playlist& playlist);

    void SkipTrack(const Playlist& playlist);


    // Sound, Sound2D, Music, Playlist
    // play, pause, resume, stop, volume, PreProcess

} // namespace magique

#endif //MAGIQUE_AUDIO_H