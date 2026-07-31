
void SetAudioBufferLooping(AudioBuffer* buffer, bool value)
{
    if (buffer != NULL)
        buffer->looping = value;
}

bool IsAudioBufferLooping(AudioBuffer* buffer) { return buffer->looping; }
