// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_AUDIO_H
#define MAGIQUE_AUDIO_H

#include <magique/fwd.hpp>

//===============================================
// Sound Module
//===============================================
// .....................................................................
// Supported Sound and Music formats: mp3, wav, flac, ogg (all raylib formats)
// Sound:       Smaller sound effects up to 10 seconds (recommended)
// Sound2D:     Spatial sound with volume regulation based on distance to the camera
// Music:       Longer soundtracks - streamed from compressed memory (should be .mp3)
// Playlist:    Aggregation of music elements

// Note: The volume for each track is relative to the global volume
// The final volume of each audio piece is the product of: MasterVolume * CategoryVolume(Sound/Music) * PlaybackVolume
// This means that the playback volume given to PlayX() function is the relative volume for the sound in that category
// .....................................................................

namespace magique
{
    //================= CONTROLS =================//

    // This scales all volume
    // SetMasterVolume() - raylib
    // GetMasterVolume() - raylib

    // Affects all Sound and Sound2D instances - clamped to (0.0 - 1.0)
    void SoundSetSoundVolume(float volume);
    float SoundGetSoundVolume();

    // Affects all Music instance including playlists - clamped to (0.0 - 1.0)
    void SoundSetMusicVolume(float volume);
    float SoundGetMusicVolume();

    //================= SOUND & SOUND2D =================//

    // Plays the given sound - multiple calls play it multiple times
    void SoundPlay(const Sound& sound, float volume = 1.0F);

    // Sets the distance at which the volume becomes 0
    void SoundSetFalloffDistance(float distance = 1000);

    // Plays the given sound attached to the given entity - volume is dynamically updated with entity position
    // Sound is automatically destroyed (stopped) if entity doesnt exist
    void SoundPlay2D(const Sound& sound, entt::entity entity, float volume = 1.0F);

    // Plays the given sound spatially at the given position
    void SoundPlay2D(const Sound& sound, Point pos, float volume = 1.0F);

    // Returns true if any instance of this sound was removed
    bool SoundStop(const Sound& sound);

    // Returns true if any instance of the given sound is currently playing
    bool SoundIsPlaying(const Sound& sound);

    //================= MUSIC =================//

    // Plays the given music - multiple calls play it multiple times
    // fade controls both fade-in and fade-out
    void MusicPlay(const Music& music, float volume = 1.0F, bool fade = true);

    // Stops the first instance of this music
    void MusicStop(const Music& music);

    //================= PLAYLIST =================//

    // Starts to play the playlist at the last played track or the beginning - volume sets the playlist volume
    // Only pass a playlist with permanent storage (either use RegisterPlaylist() or allocate it)
    // A playlist cannot be played multiple times simultaneously - plays (loops) until stopped
    void PlaylistPlay(Playlist& playlist, float volume = 1.0F);

    // Stops playing the playlist
    void PlaylistStop(Playlist& playlist);

    // Forwards the playlist manually
    // Progresses the tracks sequentially (looping) or calls the custom forward function to determine the next track
    void PlaylistForward(Playlist& playlist);

} // namespace magique

#endif // MAGIQUE_AUDIO_H
