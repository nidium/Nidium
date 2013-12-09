/*
    NativeJS Core Library
    Copyright (C) 2013 Anthony Catel <paraboul@gmail.com>
    Copyright (C) 2013 Nicolas Trani <n.trani@weelya.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef nativeutils_h__
#define nativeutils_h__

#include <stdint.h>
#include <pthread.h>

class NativeNoncopyable {
public:
    NativeNoncopyable() {}

private:
    NativeNoncopyable(const NativeNoncopyable&);
    NativeNoncopyable& operator=(const NativeNoncopyable&);
};

class NativeUtils
{
    public:
    static uint64_t getTick();
};

class NativePthreadAutoLock {
  public:
    NativePthreadAutoLock(pthread_mutex_t *mutex)
      : lock(mutex) {

        pthread_mutex_lock(lock);
    }

    ~NativePthreadAutoLock() {
        pthread_mutex_unlock(lock);
    }
  private:
    pthread_mutex_t *lock;
};

#define native_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))
#define native_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define CONST_STR_LEN(x) x, x ? sizeof(x) - 1 : 0

#endif
