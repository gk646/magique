#ifndef MAGIQUE_AUDIO_H
#define MAGIQUE_AUDIO_H

#include <magique/fwd.hpp>
#include <entt/entity/fwd.hpp>

//-----------------------------------------------
// Audio Module
//-----------------------------------------------
// .....................................................................
// Supported Sound and Music formats: mp3, wav, flac, ogg, qoa (all raylib formats)
// Note: Sound refers to shorter and music to longer tracks - music is streamed and is recommended for anything above 10s!
// Note: The volume for each track is relative to the global volume
// .....................................................................

namespace magique
{
    //----------------- SOUND -----------------//

    // Plays the given sound - multiple calls play it multiple times
    void PlaySound(const Sound& sound, float volume);

    // Returns true if any instance of the given sound is currently playing
    void IsSoundPlaying(const Sound& sound);


    //----------------- SOUND 2D -----------------//

    // Plays the given sound attached to the given entity
    // Automatically handles lifetime if entities is destroyed
    void PlaySound2D(const Sound& sound, entt::entity entity);

    // Plays the given sound attached to the given coordinates
    // IMPORTANT: Given references MUST remain valid for the full duration of the sound
    void PlaySound2D(const Sound& sound, float& x, float& y);


    //----------------- MUSIC -----------------//

    // Plays the given music
    void PlayMusic(const Music& music);

    //----------------- PLAYLIST -----------------//



    // Sound, Sound2D, Music, Playlist
    // play, pause, resume, stop, volume, PreProcess

} // namespace magique

#endif //MAGIQUE_AUDIO_H