#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <atomic>

// Wrapper for a spinlock
  struct Spinlock final
  {
      void lock()
      {
          while (flag.test_and_set(std::memory_order_acquire))
              ;
      }
      void unlock() { flag.clear(std::memory_order_release); }

  private:
      std::atomic_flag flag = ATOMIC_FLAG_INIT;
  };


#endif //SPINLOCK_H