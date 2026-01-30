// SPDX-License-Identifier: zlib-acknowledgement
#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <atomic>

// Wrapper for a spinlock
struct SpinLockGuard;
struct SpinLock final
{
    void lock()
    {
        while (locked.exchange(true, std::memory_order_acquire))
        {
            // Wait until the lock is available
        }
    }

    void unlock() { locked.store(false, std::memory_order_release); }

private:
    std::atomic<bool> locked{};
};

struct SpinLockGuard
{
    explicit SpinLockGuard(SpinLock& spinLock) : spinLock(spinLock) { spinLock.lock(); }
    ~SpinLockGuard() { spinLock.unlock(); }

    SpinLockGuard(const SpinLockGuard&) = delete;
    SpinLockGuard& operator=(const SpinLockGuard&) = delete;

private:
    SpinLock& spinLock;
};

#endif //SPINLOCK_H
