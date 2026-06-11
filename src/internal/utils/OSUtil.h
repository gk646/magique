// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_OSUTIL_H
#define MAGIQUE_OSUTIL_H

// Waits by sleeping the specified length and then busy waits until destinationTime
void WaitTime(double destinationTime, double sleepSeconds);

// Returns the amount of bytes in this process memory working set
uint64_t GetMemoryWorkingSet();

const char* OSUtilGetLocalIP();

#endif // MAGIQUE_OSUTIL_H
