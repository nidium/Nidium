/*
    NativeJS Core Library
    Copyright (C) 2014 Anthony Catel <paraboul@gmail.com>

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

#ifndef nativeargs_h__
#define nativeargs_h__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

class NativeArgs
{
public:
    class NativeArgsValue {
    public:
        NativeArgsValue() : m_Value(0LL), m_isSet(false) {
            m_ValuePtr = NULL;
        }

        void *toPtr() const {
            return m_ValuePtr;
        }
        uint64_t toInt64() const {
            return m_Value;
        }
        uint32_t toInt() const {
            return static_cast<uint32_t>(m_Value);
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
    NativeArgs() {
        m_numArgs = 8;
        m_Args = (NativeArgsValue **)malloc(sizeof(NativeArgsValue *) * m_numArgs);

        for (int i = 0; i < m_numArgs; i++) {
            m_Args[i] = new NativeArgsValue();
        }
    }
    ~NativeArgs() {
        for (int i = 0; i < m_numArgs; i++) {
            delete m_Args[i];
        }
        free(m_Args);
    }

    /*
        Overflow values are automatically allocated
    */
    NativeArgsValue& operator[] (int idx) {
        if (idx >= m_numArgs) {
            m_Args = (NativeArgsValue **)realloc(m_Args,
                sizeof(NativeArgsValue *) * (idx+1));

            for (int i = m_numArgs; i <= idx; i++) {
                m_Args[i] = new NativeArgsValue();
            }
            m_numArgs = idx+1;

        }

        return *m_Args[idx];
    }

    /*
        const version doesn't protect against overflow
    */
    const NativeArgsValue& operator[] (int idx) const {
        return *m_Args[idx];
    }

    int size() const {
        return m_numArgs;
    }
private:
    NativeArgsValue **m_Args;
    int m_numArgs;
};

#endif