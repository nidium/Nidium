/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef core_atomic_h__
#define core_atomic_h__

#include <stdint.h>

namespace Nidium {
namespace Core {
class Atomic
{
public:
#if defined(_MSC_VER)
// {{{ MSC Implementation
#include <intrin.h>
#include <WinBase.h>

// MSDN says in order to declare an interlocked function for use as an
// intrinsic, include intrin.h and put the function in a #pragma intrinsic
// directive.
// The pragma appears to be unnecessary, but doesn't hurt.
#pragma intrinsic(_InterlockedIncrement, _InterlockedExchangeAdd, \
                  _InterlockedDecrement)
#pragma intrinsic(_InterlockedCompareExchange)

    static inline int32_t Inc(int32_t *addr)
    {
        // InterlockedIncrement returns the new value, we want to return the
        // old.
        return _InterlockedIncrement(reinterpret_cast<long *>(addr)) - 1;
    };

    static inline int32_t Add(int32_t *addr, int32_t inc)
    {
        return _InterlockedExchangeAdd(reinterpret_cast<long *>(addr),
                                       static_cast<long>(inc));
    };

    static inline int32_t Dec(int32_t *addr)
    {
        // InterlockedDecrement returns the new value, we want to return the
        // old.
        return _InterlockedDecrement(reinterpret_cast<long *>(addr)) + 1;
    };

    static inline void Membar_acquire__after_atomic_dec(){};

    static inline bool Cas(int32_t *addr, int32_t before, int32_t after)
    {
        return _InterlockedCompareExchange(reinterpret_cast<long *>(addr),
                                           after, before)
               == before;
    };

    static inline void *Cas(void **addr, void *before, void *after)
    {
        return InterlockedCompareExchangePointer(addr, after, before);
    };

    static inline void Membar_acquire__after_atomic_conditional_inc(){};
// }}}
#else
    // {{{ *NIX Implementation

    static inline __attribute__((always_inline)) int32_t Inc(int32_t *addr)
    {
        return __sync_fetch_and_add(addr, 1);
    };

    static inline __attribute__((always_inline)) int32_t Add(int32_t *addr,
                                                             int32_t inc)
    {
        return __sync_fetch_and_add(addr, inc);
    };

    static inline __attribute__((always_inline)) int32_t Dec(int32_t *addr)
    {
        return __sync_fetch_and_add(addr, -1);
    };

    static inline __attribute__((always_inline)) void
    Membar_acquire__after_atomic_dec(){};

    static inline __attribute__((always_inline)) bool
    Cas(int32_t *addr, int32_t before, int32_t after)
    {
        return __sync_bool_compare_and_swap(addr, before, after);
    };

    static inline __attribute__((always_inline)) void *
    Cas(void **addr, void *before, void *after)
    {
        return __sync_val_compare_and_swap(addr, before, after);
    };

    static inline __attribute__((always_inline)) void
    Membar_acquire__after_atomic_conditional_inc(){};
// }}}
#endif
};

} // namespace Core
} // namespace Nidium

#endif
