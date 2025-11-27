/*
 * Flex Flow
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

#include <cstring>

#include "capi.hpp"
#include "grpcqueuelist.hpp"
#include "sqlitequeuelist.hpp"

#include "queuelist.h"

using namespace Model::DAO;

static IQueueList *getList(Handle h)
{
    Parent parent = hm.parent(h);
    if (parent != Parent::IQueueList) return nullptr;

    IQueueList *list = hm.get<IQueueList>(h);
    if (!list) return nullptr;

    return list;
}

extern "C"
{

u8 queuelist_init(Handle conn, Handle *out)
{
    if (!out) return 1;

    if (!hm.isNotValid(*out))
    {
        // valid handle
        return 1;
    }

    if (hm.parent(conn) != Parent::IConnect)
    {
        return 1;
    }

    IConnect *connPtr(hm.get<IConnect>(conn));
    if (!connPtr) return 1;

    IQueueList *list(nullptr);
    Type connType = hm.type(conn);
    Parent parent(Parent::IQueueList);
    Type type;
    switch (connType)
    {
    case Type::GRPCConnect:
    {
        list = new (std::nothrow) GRPCQueueList;
        type = Type::GRPCQueueList;
        break;
    }
    case Type::SQLiteConnect:
    {
        list = new (std::nothrow) SQLiteQueueList;
        type = Type::SQLiteQueueList;
        break;
    }
    default:
    {
        return 1;
    }
    }; // switch (connType)

    if (!list) return 1;

    if (list->init(connPtr))
    {
        delete list;
        return 1;
    }

    if (hm.create(out, list, parent, type))
    {
        delete list;
        return 1;
    }

    // model will own connPtr
    hm.takeOwned(conn);
    return 0;
}

u8 queuelist_destroy(Handle h)
{
    Parent parent = hm.parent(h);
    if (parent != Parent::IQueueList) return 1;

    hm.remove(h);
    return 0;
}

u8 queuelist_createQueue(Handle h, const char *name)
{
    IQueueList *list(getList(h));
    if (!list)
    {
        return 1;
    }

    return list->createQueue(name);
}

u8 queuelist_listQueue(Handle h, char **out, size_t *outSize)
{
    IQueueList *list(getList(h));
    if (!list)
    {
        return 1;
    }

    if (!outSize) return 1;

    std::vector<std::string> output;
    if (list->listQueue(output))
    {
        return 1;
    }

    *outSize = output.size();
    out = (char **)malloc(output.size() * sizeof(char *));
    if (!out) return 1;

    for (size_t i = 0; i < output.size(); ++i) {
        const char *str = output.at(i).c_str();

        out[i] = (char *)malloc((strlen(str) + 1) * sizeof(char));
        if (!out[i])
        {
            for (size_t j = 0; i <= i; ++j)
            {
                free(out[j]);
            }

            free(out);
            out = NULL;
            return 1;
        }

        strcpy(out[i], str);
    }

    return 0;
}

u8 queuelist_deleteQueue(Handle h, const char *name)
{
    IQueueList *list(getList(h));
    if (!list)
    {
        return 1;
    }

    return list->deleteQueue(name);
}

u8 queuelist_renameQueue(Handle h, const char *oldName,
                         const char *newName)
{
    IQueueList *list(getList(h));
    if (!list)
    {
        return 1;
    }

    return list->renameQueue(oldName, newName);
}

u8 queuelist_getQueue(Handle h, const char *name, Handle *out)
{
    IQueueList *list(getList(h));
    if (!list)
    {
        return 1;
    }

    IQueue *queue = list->getQueue(name);
    if (!queue) return 1;

    Type parentType(hm.type(h));
    Type type;
    switch (parentType)
    {
    case Type::GRPCQueueList:
    {
        type = Type::GRPCQueue;
        break;
    }
    case Type::SQLiteQueueList:
    {
        type = Type::SQLiteQueue;
        break;
    }
    default:
    {
        list->returnQueue(queue);
        return 1;
    }
    }; // switch (parentType)

    if (hm.create(out, queue, Parent::IQueue, type))
    {
        list->returnQueue(queue);
        return 1;
    }

    hm.takeOwned(*out);
    return 0;
}

u8 queuelist_returnQueue(Handle h, Handle queue)
{
    IQueueList *list(getList(h));
    if (!list)
    {
        return 1;
    }

    if (hm.parent(queue) != Parent::IQueue)
    {
        return 1;
    }

    IQueue *ptr = hm.get<IQueue>(queue);
    if (!ptr)
    {
        return 1;
    }

    hm.remove(queue);
    list->returnQueue(ptr);
    return 0;
}

}
