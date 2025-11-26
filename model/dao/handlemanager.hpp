/*
 * Simple Task Queue
 * Copyright (c) 2025-present fdar0536
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _MODEL_DAO_HANDLEMANAGER_HPP_
#define _MODEL_DAO_HANDLEMANAGER_HPP_

#include <vector>

#include "handle.h"

namespace Model
{

namespace DAO
{

typedef struct Entry
{
    void *ptr = nullptr;
    u16 generation = 0;
    void (*deleter)(void *) = nullptr;
    bool alive = false;
} Entry;

class HandleManager
{

public:

    ~HandleManager();

    template<class T>
    u8 create(Handle &h, T *obj)
    {
        if (!obj) return 1;

        u32 idx;
        if (!m_free_indices.empty())
        {
            idx = m_free_indices.back();
            m_free_indices.pop_back();
        }
        else
        {
            idx = (u32)m_entries.size();
            m_entries.emplace_back();
        }

        h.index      = idx;
        h.generation = m_entries[idx].generation;

        m_entries[idx].ptr   = obj;
        m_entries[idx].alive = true;
        m_entries[idx].deleter = [](void* p) { delete static_cast<T *>(p); };

        return 0;
    }

    template<class T>
    T *get(Handle h)
    {
        if (h.index >= m_entries.size()) return nullptr;

        const Entry &e = m_entries[h.index];

        if (!e.alive || e.generation != h.generation)
            return nullptr;

        return e.ptr;
    }

    void remove(Handle h);

private:

    std::vector<Entry> m_entries;

    std::vector<u32> m_free_indices;

}; // class HandleManager

} // namespace DAO

} // namespace Model

#endif // _MODEL_DAO_HANDLEMANAGER_HPP_
