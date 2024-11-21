int GetPrevDrawCalls() { return RLGL.drawCalls; }

int GetDrawCalls() { return RLGL.prevDrawCalls; }

void SwapDrawCalls()
{
    RLGL.prevDrawCalls = RLGL.drawCalls;
    RLGL.drawCalls = 0;
}