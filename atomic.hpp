#ifndef QNT_CORE_UTIL_ATOMIC_HPP
#define QNT_CORE_UTIL_ATOMIC_HPP

#if defined(__GNUC__)
  #define fetchAndIncrementP(var) __sync_fetch_and_add(var, 1)
  #define fetchAndDecrementP(var) __sync_fetch_and_add(var, -1)
  #define compareAndSwapP(var, old_val, new_val) __sync_bool_compare_and_swap(var, old_val, new_val)
  #define compareAndSwapPointerP(var, old_val, new_val) compareAndSwapP(var, old_val, new_val)
#elif defined(_MSC_VER)
  #include <windows.h>
  #define fetchAndIncrementP(var) InterlockedExchangeAdd(&var, 1)
  #define fetchAndDecrementP(var) InterlockedExchangeAdd(&var, -1)
  #define compareAndSwapP(var, old_val, new_val) InterlockedCompareExchange(var, old_val, new_val)
  #ifdef _M_AMD64
    #define compareAndSwapPointerP(var, old_val, new_val) InterlockedCompareExchange64(var, old_val, new_val)
  #else
    #define compareAndSwapPointerP(var, old_val, new_val) compareAndSwap(var, old_val, new_val)
  #endif
#else
#error "Sorry, don't know how to do atomic operations on your compiler"
#endif

#define fetchAndIncrement(var) fetchAndIncrementP(&(var))
#define fetchAndDecrement(var) fetchAndDecrementP(&(var))
#define compareAndSwap(var, old_val, new_val) compareAndSwapP(&(var), old_val, new_val)
#define compareAndSwapPointer(var, old_val, new_val) compareAndSwapPointerP(&(var), old_val, new_val)

#endif // QNT_CORE_UTIL_ATOMIC_HPP
