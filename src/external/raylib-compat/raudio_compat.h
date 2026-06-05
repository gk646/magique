#ifndef MAGIQUE_RAUDIO_COMPAT_H
#define MAGIQUE_RAUDIO_COMPAT_H

#include "raylib/raylib.h"

extern "C"
{
    void SetAudioBufferLooping(rAudioBuffer* buffer, bool value);
}



#endif // MAGIQUE_RAUDIO_COMPAT_H
