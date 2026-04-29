/*
 * Flex Flow
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

#include "model/dao/grpc/connect.hpp"
#include "model/dao/grpc/queuelist.hpp"
#include "model/dao/sqlite/connect.hpp"
#include "model/dao/sqlite/queuelist.hpp"

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

u8 consoleInit()
{
    fflush(stdout);

#ifdef _MSC_VER
#   pragma region WIN_UNICODE_SUPPORT_MAIN
#endif

#if defined _WIN32
    // change code page to UTF-8 UNICODE
    if (!IsValidCodePage(CP_UTF8))
    {
        Model::Utils::writeLastError(LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    consoleCP = GetConsoleCP();
    consoleOutputCP = GetConsoleOutputCP();

    if (!SetConsoleCP(CP_UTF8))
    {
        Model::Utils::writeLastError(LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }
    if (!SetConsoleOutputCP(CP_UTF8))
    {
        Model::Utils::writeLastError(LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    // change file stream translation mode
    if (_setmode(_fileno(stdout), _O_U8TEXT) == -1)
    {
        Model::Utils::writeLastError(LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (_setmode(_fileno(stderr), _O_U8TEXT) == -1)
    {
        Model::Utils::writeLastError(LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (_setmode(_fileno(stdin), _O_U16TEXT) == -1)
    {
        Model::Utils::writeLastError(LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }
#endif

#ifdef _MSC_VER
#   pragma endregion
#endif

    // std::ios::sync_with_stdio(false);
    return 0;
}

void consoleFin()
{
#ifdef _WIN32
    SetConsoleCP(consoleCP);
    SetConsoleOutputCP(consoleOutputCP);
#endif
}

u8 spdlogInit(const std::string &path)
{
    spdlog::debug("{}:{} spdlogInit", LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("{}:{} path is: {}", LOG_FILE_PATH(__FILE__), __LINE__, path);
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
        spdlog::error("{}:{} Fail to initialize spdlog, path is: {}",
                      LOG_FILE_PATH(__FILE__), __LINE__, path);
        return 1;
    }

    return 0;
}

u8 sqliteInit(Model::DAO::IQueueList **out, const std::string &target)
{
    spdlog::debug("{}:{} sqliteInit", LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("{}:{} target is: {}", LOG_FILE_PATH(__FILE__), __LINE__, target);

    Model::DAO::SQLite::Connect *conn(nullptr);
    conn = new (std::nothrow) Model::DAO::SQLite::Connect();
    if (!conn)
    {
        spdlog::error("{}:{} Fail to allocate memory",
                      LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (conn->startConnect(target))
    {
        delete conn;
        spdlog::error("{}:{} Fail to connect to {}",
                      LOG_FILE_PATH(__FILE__), __LINE__, target);
        return 1;
    }

    Model::DAO::SQLite::QueueList *sqlPtr;
    try
    {
        sqlPtr = new Model::DAO::SQLite::QueueList;
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to allocate memory",
                      LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (sqlPtr->init(conn))
    {
        spdlog::error("{}:{} Fail to initialize sqlite queue list",
                      LOG_FILE_PATH(__FILE__), __LINE__);
        delete sqlPtr;
        delete conn;
        return 1;
    }

    *out = sqlPtr;
    return 0;
}

u8 grpcInit(Model::DAO::IQueueList **out, const std::string &target, const i32 port)
{
    spdlog::debug("{}:{} grpcInit", LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("{}:{} target is: {}", LOG_FILE_PATH(__FILE__), __LINE__, target);
    spdlog::debug("{}:{} port is: {}", LOG_FILE_PATH(__FILE__), __LINE__, port);
    
    Model::DAO::GRPC::Connect *conn = new (std::nothrow) Model::DAO::GRPC::Connect;
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

    if (conn->startConnect(target, port))
    {
        delete conn;
        spdlog::error("{}:{} Fail to connect to server", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    Model::DAO::GRPC::QueueList *queueList = new (std::nothrow) Model::DAO::GRPC::QueueList;
    if (!queueList)
    {
        delete conn;
        spdlog::error("{}:{} Fail to allocate memory", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (queueList->init(conn))
    {
        delete queueList;
        spdlog::error("{}:{} Fail to initialize queue list", LOG_FILE_PATH(__FILE__), __LINE__);
        delete conn;
        return 1;
    }

    *out = queueList;
    return 0;
}

} // end namespace Global

} // end namespace Controller
