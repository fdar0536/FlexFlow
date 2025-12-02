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

#include "handlemanager.hpp"

namespace Model
{

namespace DAO
{

HandleManager hm;

HandleManager::~HandleManager()
{
    for (size_t i = 0; i < m_entries.size(); ++i)
    {
        Entry &e = m_entries.at(i);
        if (e.ptr)
        {
            if (e.deleter) e.deleter(e.ptr);
        }

    } // for (size_t i = 0; i < m_entries.size(); ++i)
}

bool HandleManager::isNotValid(Handle h)
{
    uint32_t idx = HANDLE_GET_IDX(h);
    if (idx >= m_entries.size()) return true;

    const Entry &e = m_entries.at(idx);

    u16 gen = HANDLE_GET_GEN(h);
    if (!e.alive || e.generation != gen)
        return true;

    return false;
}

u8 HandleManager::takeOwned(Handle h)
{
    if (isNotValid(h))
    {
        return 1;
    }

    Entry &e = m_entries.at(HANDLE_GET_IDX(h));
    e.deleter = nullptr;

    return 0;
}

Parent HandleManager::parent(Handle h)
{
    if (isNotValid(h))
    {
        return Parent::invaild;
    }

    const Entry &e = m_entries.at(HANDLE_GET_IDX(h));
    return e.parent;
}

Type HandleManager::type(Handle h)
{
    if (isNotValid(h))
    {
        return Type::invaild;
    }

    const Entry &e = m_entries.at(HANDLE_GET_IDX(h));
    return e.type;
}

void HandleManager::remove(Handle h)
{
    if (isNotValid(h))
    {
        return;
    }

    uint32_t idx = HANDLE_GET_IDX(h);
    Entry &e = m_entries.at(idx);

    if (e.deleter) e.deleter(e.ptr);
    e.ptr   = nullptr;
    e.alive = false;
    e.generation++;

    // to avoid overflow
    if (e.generation > 4095) e.generation = 0;

    m_free_indices.push_back(idx);
}

} // namespace DAO

} // namespace Model
