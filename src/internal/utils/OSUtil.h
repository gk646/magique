// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_OSUTIL_H
#define MAGIQUE_OSUTIL_H

// Waits by sleeping the specified length and then busy waits until destinationTime
void WaitTime(double destinationTime, double sleepSeconds);

// Sets given threads priority
void SetupThreadPriority(int thread, bool high = true);

// Sets the current process priority to high
void SetupProcessPriority();

// Returns the amount of bytes in this process memory working set
uint64_t GetMemoryWorkingSet();

#endif //MAGIQUE_OSUTIL_H