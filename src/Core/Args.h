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
    class ArgsValue
    {
    public:
        ArgsValue() : m_Value(0LL), m_isSet(false)
        {
            m_ValuePtr = NULL;
        }

        void *toPtr() const
        {
            return m_ValuePtr;
        }

        int64_t toInt64() const
        {
            return m_Value;
        }

        int32_t toInt() const
        {
            return static_cast<int32_t>(m_Value);
        }

        bool toBool() const
        {
            return static_cast<bool>(m_Value);
        }

        bool isSet() const
        {
            return m_isSet;
        }

        void set(void *val)
        {
            m_isSet    = true;
            m_ValuePtr = val;
        }

        void set(int64_t val)
        {
            m_isSet = true;
            m_Value = val;
        }

    private:
        union
        {
            uint64_t m_Value;
            void *m_ValuePtr;
        };

        bool m_isSet;
    };

    Args() : m_FillArgs(0)
    {
        m_NumArgs = 10;
        m_Args    = new ArgsValue[m_NumArgs];
    }

    ~Args()
    {
        delete[] m_Args;
    }

    ArgsValue &operator[](int idx)
    {
        if (idx >= m_FillArgs) {
            m_FillArgs = idx + 1;
        }

        if (idx >= m_NumArgs) {
            ndm_log(NDM_LOG_ERROR, "Args", "Args overflow");
            *(volatile int *)0 = 42;
        }

        return m_Args[idx];
    }

    /*
        Const version protect against overflow
    */
    const ArgsValue &operator[](int idx) const
    {
        if (idx >= m_NumArgs) {
            ndm_log(NDM_LOG_ERROR, "Args",
                "/!\\ Overflow in accessing Args value. Begining of the array "
                "returned");
            return m_Args[0];
        }
        return m_Args[idx];
    }

    int size() const
    {
        return m_FillArgs;
    }

private:
    ArgsValue *m_Args;
    int m_NumArgs;
    int m_FillArgs;
};

} // namespace Core
} // namespace Nidium

#endif
