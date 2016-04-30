/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef core_args_h__
#define core_args_h__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

namespace Nidium {
namespace Core {

class Args
{
public:
    class ArgsValue {
    public:
        ArgsValue() : m_Value(0LL), m_isSet(false) {
            m_ValuePtr = NULL;
        }

        void *toPtr() const {
            return m_ValuePtr;
        }
        int64_t toInt64() const {
            return m_Value;
        }
        int32_t toInt() const {
            return static_cast<int32_t>(m_Value);
        }

        bool toBool() const {
            return (bool)m_Value;
        }

        bool isSet() const {
            return m_isSet;
        }

        void set(void *val) {
            m_isSet = true;
            m_ValuePtr = val;
        }
        void set(int64_t val) {
            m_isSet = true;
            m_Value = val;
        }
    private:
        union {
            uint64_t m_Value;
            void *m_ValuePtr;
        };

        bool m_isSet;
    };
    Args() {
        m_numArgs = 8;
        m_Args = (ArgsValue **)malloc(sizeof(ArgsValue *) * m_numArgs);

        for (int i = 0; i < m_numArgs; i++) {
            m_Args[i] = new ArgsValue();
        }
    }
    ~Args() {
        for (int i = 0; i < m_numArgs; i++) {
            delete m_Args[i];
        }
        free(m_Args);
    }

    /*
        Overflow values are automatically allocated
    */
    ArgsValue& operator[] (int idx) {
        if (idx >= m_numArgs) {
            m_Args = (ArgsValue **)realloc(m_Args, sizeof(ArgsValue *) * (idx+1));

            for (int i = m_numArgs; i <= idx; i++) {
                m_Args[i] = new ArgsValue();
            }
            m_numArgs = idx+1;

        }

        return *m_Args[idx];
    }

    /*
        const version doesn't protect against overflow
    */
    const ArgsValue& operator[] (int idx) const {
        if (idx >= m_numArgs) {
            printf("/!\\ Overflow in accessing Args value. Beggining of the array returned\n");
            return *m_Args[0];
        }
        return *m_Args[idx];
    }

    int size() const {
        return m_numArgs;
    }
private:
    ArgsValue **m_Args;
    int m_numArgs;
};

} // namespace Core
} // namespace Nidium

#endif

