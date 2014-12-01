#ifndef nativeatom_h__
#define nativeatom_h__

#include <stdint.h>

#if defined(_MSC_VER)
    #include <intrin.h>

    //MSDN says in order to declare an interlocked function for use as an
    //intrinsic, include intrin.h and put the function in a #pragma intrinsic
    //directive.
    //The pragma appears to be unnecessary, but doesn't hurt.
    #pragma intrinsic(_InterlockedIncrement, _InterlockedExchangeAdd, _InterlockedDecrement)
    #pragma intrinsic(_InterlockedCompareExchange)

    static inline int32_t native_atomic_inc(int32_t* addr) {
        // InterlockedIncrement returns the new value, we want to return the old.
        return _InterlockedIncrement(reinterpret_cast<long*>(addr)) - 1;
    }

    static inline int32_t native_atomic_add(int32_t* addr, int32_t inc) {
        return _InterlockedExchangeAdd(reinterpret_cast<long*>(addr), static_cast<long>(inc));
    }

    static inline int32_t native_atomic_dec(int32_t* addr) {
        // InterlockedDecrement returns the new value, we want to return the old.
        return _InterlockedDecrement(reinterpret_cast<long*>(addr)) + 1;
    }

    static inline void native_membar_acquire__after_atomic_dec() { }

    static inline bool native_atomic_cas(int32_t* addr, int32_t before, int32_t after) {
        return _InterlockedCompareExchange(reinterpret_cast<long*>(addr), after, before) == before;
    }

    static inline void* native_atomic_cas(void** addr, void* before, void* after) {
        return InterlockedCompareExchangePointer(addr, after, before);
    }

    static inline void native_membar_acquire__after_atomic_conditional_inc() { }

#else

    static inline __attribute__((always_inline)) int32_t native_atomic_inc(int32_t* addr) {
        return __sync_fetch_and_add(addr, 1);
    }

    static inline __attribute__((always_inline)) int32_t native_atomic_add(int32_t* addr, int32_t inc) {
        return __sync_fetch_and_add(addr, inc);
    }

    static inline __attribute__((always_inline)) int32_t native_atomic_dec(int32_t* addr) {
        return __sync_fetch_and_add(addr, -1);
    }

    static inline __attribute__((always_inline)) void native_membar_acquire__after_atomic_dec() { }

    static inline __attribute__((always_inline)) bool native_atomic_cas(int32_t* addr,
                                                                    int32_t before,
                                                                    int32_t after) {
        return __sync_bool_compare_and_swap(addr, before, after);
    }

    static inline __attribute__((always_inline)) void* native_atomic_cas(void** addr,
                                                                     void* before,
                                                                     void* after) {
        return __sync_val_compare_and_swap(addr, before, after);
    }

    static inline __attribute__((always_inline)) void native_membar_acquire__after_atomic_conditional_inc() { }

#endif

#endif
