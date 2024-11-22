int GetPrevDrawCalls() { return RLGL.drawCalls; }

int GetDrawCalls() { return RLGL.prevDrawCalls; }

void ResetDrawCallCount()
{
    RLGL.prevDrawCalls = RLGL.drawCalls;
    RLGL.drawCalls = 0;
}