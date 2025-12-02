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

typedef enum class Parent
{
    invaild,
    IConnect,
    IQueueList,
    IQueue
} Parent;

typedef enum class Type
{
    invaild,
    GRPCConnect,
    GRPCQueueList,
    GRPCQueue,
    SQLiteConnect,
    SQLiteQueueList,
    SQLiteQueue
} Type;

typedef struct Entry
{
    void *ptr = nullptr;
    u16 generation = 0;
    void (*deleter)(void *) = nullptr;
    bool alive = false;
    Parent parent = Parent::invaild;
    Type type = Type::invaild;
} Entry;

class HandleManager;

extern HandleManager hm;

class HandleManager
{

public:

    ~HandleManager();

    template<class T>
    u8 create(Handle *h, T *obj, Parent parent, Type type)
    {
        if (!obj || !h) return 1;

        u32 idx;
        if (!m_free_indices.empty())
        {
            idx = m_free_indices.back();
            m_free_indices.pop_back();
        }
        else
        {
            idx = static_cast<u32>(m_entries.size());
            m_entries.emplace_back();
        }

        h->index      = idx;
        h->generation = m_entries[idx].generation;

        m_entries.at(idx).ptr   = obj;
        m_entries.at(idx).alive = true;
        m_entries.at(idx).deleter = [](void* p) { delete static_cast<T *>(p); };
        m_entries.at(idx).parent = parent;
        m_entries.at(idx).type = type;

        return 0;
    }

    bool isNotValid(Handle h);

    u8 takeOwned(Handle h);

    Parent parent(Handle h);

    Type type(Handle h);

    template<class T>
    T *get(Handle h)
    {
        if (isNotValid(h))
        {
            return nullptr;
        }

        const Entry &e = m_entries.at(h.index);
        return static_cast<T *>(e.ptr);
    }

    void remove(Handle h);

private:

    std::vector<Entry> m_entries;

    std::vector<u32> m_free_indices;

}; // class HandleManager

} // namespace DAO

} // namespace Model

#endif // _MODEL_DAO_HANDLEMANAGER_HPP_
