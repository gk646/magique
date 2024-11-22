#ifndef RLGL_COMPAT_H
#define RLGL_COMPAT_H

extern "C" {

int GetPrevDrawCalls();

int GetDrawCalls();

void ResetDrawCallCount();

}

#endif // RLGL_COMPAT_H