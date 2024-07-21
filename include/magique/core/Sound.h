#ifndef MAGIQUE_AUDIO_H
#define MAGIQUE_AUDIO_H

#include <magique/fwd.hpp>
#include <entt/entity/fwd.hpp>
#include <magique/assets/types/Playlist.h>

//-----------------------------------------------
// Sound Module
//-----------------------------------------------
// .....................................................................
// Supported Sound and Music formats: mp3, wav, flac, ogg (all raylib formats)
// Sound:       Smaller sound effects up to 10 seconds (recommended)
// Sound2D:     Spatial sound with volume regulation based on camera
// Music:       Longer sound tracks - streamed from compressed memory (should be .mp3)
// Playlist:    Aggregation of music elements

// Note: The volume for each track is relative to the global volume
// The final volume of each audio piece is the product of: MasterVolume * CategoryVolume(Sound/Music) * PlaybackVolume
// .....................................................................

namespace magique
{
    //----------------- CONTROLS -----------------//

    // This scales all volume
    // SetMasterVolume() - raylib
    // GetMasterVolume() - raylib

    // Affects all Sound and Sound2D instances - has to be between (0.0 - 1.0)
    void SetSoundVolume(float volume);

    // Returns the current sound volume - (0.0 - 1.0)
    float GetSoundVolume();

    // Affects all Music instance including playlists - has to be between (0.0 - 1.0)
    void SetMusicVolume(float volume);

    // Returns the current music volume - (0.0 - 1.0)
    float GetMusicVolume();

    //----------------- SOUND & SOUND2D -----------------//

    // Plays the given sound - multiple calls play it multiple times
    void PlaySound(const Sound& sound, float volume = 1.0F);

    // Plays the given sound attached to the given entity
    // Automatically handles lifetime if entities is destroyed
    void PlaySound2D(const Sound& sound, entt::entity entity, float volume = 1.0F);

    // Plays the given sound attached to the given coordinates
    // IMPORTANT: Given references MUST remain valid for the full duration of the sound
    void PlaySound2D(const Sound& sound, float& x, float& y, float volume = 1.0F);

    // Stops playing the first instance of this sound
    // Checks both normal and 2D
    void StopSound(const Sound& sound);

    // Returns true if any instance of the given sound is currently playing
    // Checks both normal and 2D
    bool IsSoundPlaying(const Sound& sound);

    //----------------- MUSIC -----------------//

    // Plays the given music - multiple calls play it multiple times
    // fade controls both fade-in and fade-out
    void PlayMusic(const Music& music, float volume = 1.0F, bool fade = true);

    // Stops the first instance of this music
    void StopMusic(const Music& music);

    //----------------- PLAYLIST -----------------//

    // Starts to play the playlist at the last played track or the beginning
    // Only pass a playlist with permament storage (either use RegisterPlaylist() or allocate it)
    // A playlist cannot be played multiple times simultanously - plays (loops) until stopped
    void PlayPlaylist(Playlist* playlist);

    // Stops playing the playlist
    void StopPlaylist(Playlist& playlist);

    // Forwards the playlist manually
    // Progresses the tracks sequentially (looping) or calls the custom forward function to determine the next track
    void ForwardPlaylist(Playlist& playlist);

    // Sound, Sound2D, Music, Playlist
    // play, pause, resume, stop, volume, PreProcess

} // namespace magique

#endif //MAGIQUE_AUDIO_H