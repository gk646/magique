#ifndef MAGIQUE_AUDIOPLAYER_H
#define MAGIQUE_AUDIOPLAYER_H

#include <magique/fwd.hpp>
#include <entt/entity/fwd.hpp>

// Supported Sound and Music formats: mp3, wav, flac, ogg, qoa
// Note: Sound refers to shorter and music to longer tracks - music is streamed and is recommended for anything above 10s!
// Note: The volume for each track is relative to the global volume

namespace magique
{
    // Plays the given sound - multiple calls play it multiple times
    void PlaySound(const Sound& sound, float volume);

    // Returns true if any instance of the given sound is currently playing
    void IsSoundPlaying(const Sound& sound);


    // Plays the given music
    void PlayMusic(const Music& music);


    // Plays the given sound attached to the given entity
    // Automatically handles lifetime if entities is destroyed
    void PlaySound2D(const Sound& sound, entt::entity entity);

    // Plays the given sound attached to the given coordinates
    // IMPORTANT: Given references MUST remain valid for the full duration of the sound
    void PlaySound2D(const Sound& sound, float& x, float& y);


    // Sound, Sound2D, Music, Playlist
    // play, pause, resume, stop, volume, PreProcess

} // namespace magique

#endif //MAGIQUE_AUDIOPLAYER_H