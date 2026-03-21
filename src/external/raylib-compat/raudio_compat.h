#ifndef STARFIGHTER_RAUDIO_COMPAT_H
#define STARFIGHTER_RAUDIO_COMPAT_H

#include "raylib/raylib.h"

extern "C"
{
    void SetAudioBufferLooping(rAudioBuffer* buffer, bool value);
}



#endif // STARFIGHTER_RAUDIO_COMPAT_H
