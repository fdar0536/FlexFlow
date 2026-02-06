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

#include <new>
#include "handlemanager.hpp"
#include "grpcconnect.hpp"
#include "sqliteconnect.hpp"
#include "connect.h"

using namespace Model::DAO;

static IConnect *getConn(Handle h)
{
    spdlog::debug("{}:{} getConn", LOG_FILE_PATH(__FILE__), __LINE__);

    Parent parent = hm.parent(h);
    if (parent != Parent::IConnect)
    {
        spdlog::error("{}:{} parent is not IConnect",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return nullptr;
    }

    IConnect *conn = hm.get<IConnect>(h);
    if (!conn)
    {
        spdlog::error("{}:{} Fail to get IConnect",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return nullptr;
    }

    return conn;
}

extern "C"
{

u8 connect_init(Handle *out, u8 backend)
{
    if (!out)
    {
        spdlog::error("{}:{} out is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (!hm.isNotValid(*out))
    {
        spdlog::error("{}:{} out is valid handle",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (backend > 1)
    {
        spdlog::error("{}:{} invalid backend",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    Type type;
    IConnect *conn(nullptr);
    switch (backend)
    {
    case BACKEND_GRPC:
    {
        conn = new (std::nothrow) GRPCConnect;
        type = Type::GRPCConnect;
        break;
    }
    case BACKEND_SQLITE:
    {
        conn = new (std::nothrow) SQLiteConnect;
        type = Type::SQLiteConnect;
        break;
    }
    default:
    {
        spdlog::error("{}:{} invalid backend",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }
    } // switch (backend)

    if (!conn)
    {
        spdlog::error("{}:{} Fail to allocate memory",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (conn->init())
    {
        delete conn;
        spdlog::error("{}:{} Fail to initialize conn",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (hm.create(out, conn, Parent::IConnect, type))
    {
        delete conn;
        spdlog::error("{}:{} Fail to create handle",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

u8 connect_destroy(Handle h)
{
    spdlog::debug("{}:{} connect_destroy", LOG_FILE_PATH(__FILE__), __LINE__);

    Parent parent = hm.parent(h);
    if (parent != Parent::IConnect)
    {
        spdlog::error("{}:{} parent is not IConnect",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    hm.remove(h);
    return 0;
}

u8 connect_startConnect(Handle h, const char *target, const i32 port)
{
    spdlog::debug("{}:{} connect_startConnect", LOG_FILE_PATH(__FILE__), __LINE__);

    IConnect *conn = getConn(h);
    if (!conn)
    {
        spdlog::error("{}:{} Fail to get IConnect",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return conn->startConnect(target, port);;
}

void *connect_connectToken(Handle h)
{
    spdlog::debug("{}:{} connect_connectToken", LOG_FILE_PATH(__FILE__), __LINE__);

    IConnect *conn = getConn(h);
    if (!conn)
    {
        spdlog::error("{}:{} Fail to get IConnect",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return NULL;
    }

    return conn->connectToken();
}

u8 connect_targetPath(Handle h, char *buf, size_t *bufSize)
{
    spdlog::debug("{}:{} connect_targetPath", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!buf && !bufSize)
    {
        spdlog::error("{}:{} both buf and bufSize are nullptr",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    IConnect *conn = getConn(h);
    if (!conn)
    {
        spdlog::error("{}:{} Fail to get IConnect",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (!buf)
    {
        // bufSize is not null here
        *bufSize = conn->targetPath().size();
        return 0;
    }

    // buf is not null from here
    if (!bufSize)
    {
        spdlog::error("{}:{} bufSize is nullptr",
            LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
    }

    if (*bufSize <= conn->targetPath().size())
    {
        spdlog::error("{}:{} bufSize is too small",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    // *bufSize >= conn->targetPath().size()
    memcpy(buf, conn->targetPath().c_str(),
           conn->targetPath().size() * sizeof(size_t));
    return 0;
}

} // extern "C"
