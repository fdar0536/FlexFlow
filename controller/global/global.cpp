/*
 * Simple Task Queue
 * Copyright (c) 2024 fdar0536
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

#include <cstdio>

#include "model/dao/grpcconnect.hpp"
#include "model/dao/grpcqueuelist.hpp"
#include "model/dao/sqliteconnect.hpp"
#include "model/dao/sqlitequeuelist.hpp"
#include "model/utils.hpp"

#ifdef _WIN32
#include "windows.h"
#include "io.h"
#include "fcntl.h"
#endif

#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"

#include "global.hpp"

namespace Controller
{

namespace Global
{

#ifdef _WIN32
static UINT consoleCP(0);

static UINT consoleOutputCP(0);
#endif

FF_MODEL_API u8 consoleInit()
{
    fflush(stdout);

#ifdef _MSC_VER
#   pragma region WIN_UNICODE_SUPPORT_MAIN
#endif

#if defined _WIN32
    // change code page to UTF-8 UNICODE
    if (!IsValidCodePage(CP_UTF8))
    {
        Model::Utils::writeLastError(__FILE__, __LINE__);
        return 1;
    }

    consoleCP = GetConsoleCP();
    consoleOutputCP = GetConsoleOutputCP();

    if (!SetConsoleCP(CP_UTF8))
    {
        Model::Utils::writeLastError(__FILE__, __LINE__);
        return 1;
    }
    if (!SetConsoleOutputCP(CP_UTF8))
    {
        Model::Utils::writeLastError(__FILE__, __LINE__);
        return 1;
    }

    // change file stream translation mode
    if (_setmode(_fileno(stdout), _O_U8TEXT) == -1)
    {
        Model::Utils::writeLastError(__FILE__, __LINE__);
        return 1;
    }

    if (_setmode(_fileno(stderr), _O_U8TEXT) == -1)
    {
        Model::Utils::writeLastError(__FILE__, __LINE__);
        return 1;
    }

    if (_setmode(_fileno(stdin), _O_U16TEXT) == -1)
    {
        Model::Utils::writeLastError(__FILE__, __LINE__);
        return 1;
    }
#endif

#ifdef _MSC_VER
#   pragma endregion
#endif

    // std::ios::sync_with_stdio(false);
    return 0;
}

FF_MODEL_API void consoleFin()
{
#ifdef _WIN32
    SetConsoleCP(consoleCP);
    SetConsoleOutputCP(consoleOutputCP);
#endif
}

FF_MODEL_API bool isAdmin()
{
#ifdef _WIN32
    PSID sid;
    SID_IDENTIFIER_AUTHORITY auth = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&auth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0,
                                  &sid))
    {
        return true;
    }

    BOOL res;
    if (!CheckTokenMembership(nullptr, sid, &res))
    {
        return true;
    }

    FreeSid(sid);
    return res;
#else
    if (!system("sudo -v -n &>/dev/null"))
    {
        // system("sudo -v -n &>/dev/null") == 0
        return true;
    }

    return (geteuid() == 0);
#endif
}

FF_MODEL_API u8 spdlogInit(const std::string &path)
{
    if (path.empty())
    {
        return 0;
    }

    try
    {
        auto daily_logger = spdlog::daily_logger_mt(
            "STQLog", path);
        spdlog::set_default_logger(daily_logger);
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to initialize spdlog, path is: {}", __FILE__, __LINE__,
        path);
        return 1;
    }

    return 0;
}

FF_MODEL_API u8 sqliteInit(Model::DAO::IQueueList *out, const std::string &target)
{
    Model::DAO::SQLiteConnect *conn(nullptr);
    try
    {
        conn = new Model::DAO::SQLiteConnect();
        if (!conn)
        {
            spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
            return 1;
        }
    }
    catch(...)
    {
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return 1;
    }

    if (conn->startConnect(target))
    {
        delete conn;
        spdlog::error("{}:{} Fail to initialize sqlite", __FILE__, __LINE__);
        return 1;
    }

    Model::DAO::SQLiteQueueList *sqlPtr;
    try
    {
        sqlPtr = new Model::DAO::SQLiteQueueList;
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return 1;
    }

    if (sqlPtr->init(conn))
    {
        spdlog::error("{}:{} Fail to initialize sqlite queue list", __FILE__, __LINE__);
        delete sqlPtr;
        delete conn;
        return 1;
    }

    out = sqlPtr;
    return 0;
}

FF_MODEL_API u8 grpcInit(Model::DAO::IQueueList *out, const std::string &target, const i32 port)
{
    Model::DAO::GRPCConnect *conn = new (std::nothrow) Model::DAO::GRPCConnect;
    if (!conn)
    {
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return 1;
    }

    if (conn->init())
    {
        delete conn;
        spdlog::error("{}:{} Fail to initialize conn", __FILE__, __LINE__);
        return 1;
    }

    if (conn->startConnect(target, port))
    {
        delete conn;
        spdlog::error("{}:{} Fail to connect to server", __FILE__, __LINE__);
        return 1;
    }

    Model::DAO::GRPCQueueList *queueList = new (std::nothrow) Model::DAO::GRPCQueueList;
    if (!queueList)
    {
        delete conn;
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return 1;
    }

    if (queueList->init(conn))
    {
        delete queueList;
        spdlog::error("{}:{} Fail to initialize queue list", __FILE__, __LINE__);
        delete conn;
        return 1;
    }

    out = queueList;
    return 0;
}

} // end namesapce Global

} // end namespace Controller
