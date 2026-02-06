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

#include "handlemanager.hpp"
#include "grpcqueuelist.hpp"
#include "sqlitequeuelist.hpp"

#include "queuelist.h"

using namespace Model::DAO;

static IQueueList *getList(Handle h)
{
    spdlog::debug("{}:{} getList", LOG_FILE_PATH(__FILE__), __LINE__);

    Parent parent = hm.parent(h);
    if (parent != Parent::IQueueList)
    {
        spdlog::error("{}:{} parent is not IQueueList",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return nullptr;
    }

    IQueueList *list = hm.get<IQueueList>(h);
    if (!list)
    {
        spdlog::error("{}:{} Fail to get IQueueList",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return nullptr;
    }

    return list;
}

extern "C"
{

u8 queuelist_init(Handle conn, Handle *out)
{
    spdlog::debug("{}:{} queuelist_init", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!out)
    {
        spdlog::error("{}:{} out is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (!hm.isNotValid(*out))
    {
        // valid handle
        spdlog::error("{}:{} out is valid handle",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (hm.parent(conn) != Parent::IConnect)
    {
        spdlog::error("{}:{} parent is not IConnect",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    IConnect *connPtr(hm.get<IConnect>(conn));
    if (!connPtr)
    {
        spdlog::error("{}:{} Fail to get IConnect",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

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
        spdlog::error("{}:{} invalid connType",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }
    }; // switch (connType)

    if (!list)
    {
        spdlog::error("{}:{} Fail to allocate memory",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (list->init(connPtr))
    {
        delete list;
        spdlog::error("{}:{} Fail to initialize list",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (hm.create(out, list, parent, type))
    {
        delete list;
        spdlog::error("{}:{} Fail to create handle",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    // model will own connPtr
    hm.takeOwned(conn);
    return 0;
}

u8 queuelist_destroy(Handle h)
{
    spdlog::debug("{}:{} queuelist_destroy", LOG_FILE_PATH(__FILE__), __LINE__);

    Parent parent = hm.parent(h);
    if (parent != Parent::IQueueList)
    {
        spdlog::error("{}:{} parent is not IQueueList",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    hm.remove(h);
    return 0;
}

u8 queuelist_createQueue(Handle h, const char *name)
{
    spdlog::debug("{}:{} queuelist_createQueue", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!name)
    {
        spdlog::error("{}:{} name is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    spdlog::debug("{}:{} name: {}", LOG_FILE_PATH(__FILE__), __LINE__, name);

    IQueueList *list(getList(h));
    if (!list)
    {
        spdlog::error("{}:{} Fail to get IQueueList",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return list->createQueue(name);
}

u8 queuelist_listQueue(Handle h, char ***out, size_t *outSize)
{
    spdlog::debug("{}:{} queuelist_listQueue", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!out || !outSize)
    {
        spdlog::error("{}:{} out or outSize is nullptr",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    IQueueList *list(getList(h));
    if (!list)
    {
        spdlog::error("{}:{} Fail to get IQueueList",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    std::vector<std::string> output;
    if (list->listQueue(output))
    {
        spdlog::error("{}:{} Fail to list queue",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (!output.size())
    {
        *outSize = 0;
        return 0;
    }

    *outSize = output.size();
    *out = (char **)malloc((*outSize) * sizeof(char *));
    if (!out) return 1;

    for (size_t i = 0; i < output.size(); ++i) {
        const char *str = output.at(i).c_str();

        *out[i] = (char *)calloc((strlen(str) + 1), sizeof(char));
        if (!out[i])
        {
            spdlog::error("{}:{} Fail to calloc",
                LOG_FILE_PATH(__FILE__), __LINE__);

            for (size_t j = 1; j <= i; ++j)
            {
                free(out[j]);
            }

            if (i != 0) free(out[0]);

            free(out);
            return 1;
        }

        snprintf(*out[i], strlen(str) + 1, "%s", str);
    }

    return 0;
}

u8 queuelist_deleteQueue(Handle h, const char *name)
{
    spdlog::debug("{}:{} queuelist_deleteQueue", LOG_FILE_PATH(__FILE__), __LINE__);

    IQueueList *list(getList(h));
    if (!list)
    {
        spdlog::error("{}:{} Fail to get IQueueList",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return list->deleteQueue(name);
}

u8 queuelist_renameQueue(Handle h, const char *oldName,
                         const char *newName)
{
    spdlog::debug("{}:{} queuelist_renameQueue", LOG_FILE_PATH(__FILE__), __LINE__);

    IQueueList *list(getList(h));
    if (!list)
    {
        spdlog::error("{}:{} Fail to get IQueueList",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return list->renameQueue(oldName, newName);
}

u8 queuelist_getQueue(Handle h, const char *name, Handle *out)
{
    spdlog::debug("{}:{} queuelist_getQueue", LOG_FILE_PATH(__FILE__), __LINE__);

    IQueueList *list(getList(h));
    if (!list)
    {
        spdlog::error("{}:{} Fail to get IQueueList",
            LOG_FILE_PATH(__FILE__), __LINE__);
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
        spdlog::error("{}:{} invalid parent type",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }
    }; // switch (parentType)

    if (hm.create(out, queue, Parent::IQueue, type))
    {
        spdlog::error("{}:{} Fail to create handle",
            LOG_FILE_PATH(__FILE__), __LINE__);
        list->returnQueue(queue);
        return 1;
    }

    hm.takeOwned(*out);
    return 0;
}

u8 queuelist_returnQueue(Handle h, Handle queue)
{
    spdlog::debug("{}:{} queuelist_returnQueue", LOG_FILE_PATH(__FILE__), __LINE__);

    IQueueList *list(getList(h));
    if (!list)
    {
        spdlog::error("{}:{} Fail to get IQueueList",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (hm.parent(queue) != Parent::IQueue)
    {
        spdlog::error("{}:{} queue is not IQueue",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    IQueue *ptr = hm.get<IQueue>(queue);
    if (!ptr)
    {
        spdlog::error("{}:{} Fail to get IQueue",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    hm.remove(queue);
    list->returnQueue(ptr);
    return 0;
}

} // extern "C"
