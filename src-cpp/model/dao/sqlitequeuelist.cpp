/*
 * Flex Flow
 * Copyright (c) 2023-present fdar0536
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

#include <filesystem>
#include <new>

#include "spdlog/spdlog.h"

#include "controller/global/global.hpp"
#include "model/errmsg.hpp"

#include "sqlitequeue.hpp"
#include "sqlitequeuelist.hpp"
#include "dirutils.hpp"

#ifdef _WIN32
#include "model/proc/winproc.hpp"
#elif (defined __linux__)
#include "model/proc/linuxproc.hpp"
#else
#include "model/proc/macproc.hpp"
#endif

namespace Model
{

namespace DAO
{

SQLiteQueueList::SQLiteQueueList()
{
    m_conn =nullptr;
}

SQLiteQueueList::~SQLiteQueueList()
{
    if (m_conn) delete m_conn;
}

u8
SQLiteQueueList::init(IConnect *connect)
{
    spdlog::debug("{}:{} SQLiteQueueList::init", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!connect)
    {
        spdlog::error("{}:{} \"connect\" is nullptr.", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_INVALID_ARGUMENT;
    }

    if (DirUtils::verifyDir(connect->targetPath()))
    {
        spdlog::error("{}:{} Fail to verify target path.", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_INVALID_ARGUMENT;
    }

    // create queue
    std::error_code ec;
    std::string fileName;
    m_queueList.clear();
    m_conn = connect;
    for (const auto& entry : std::filesystem::directory_iterator(connect->targetPath()))
    {
        if (std::filesystem::is_directory(entry))
        {
            spdlog::warn("{}:{} {} is directory, ignore...", LOG_FILE_PATH(__FILE__), __LINE__,
                         entry.path().string());
            continue;
        }

        if (!std::filesystem::is_regular_file(entry))
        {
            spdlog::warn("{}:{} {} is not regular file, ignore...",
                LOG_FILE_PATH(__FILE__), __LINE__,
                entry.path().string());
            continue;
        }

        // regular file
        fileName = entry.path().string();
        DirUtils::convertPath(fileName);
        size_t index = fileName.find_last_of("/");
        std::string name = fileName.substr(index + 1);
        index = name.find_last_of(".");
        if (name.substr(index + 1) != "db")
        {
            spdlog::warn("{}:{} {} is not database, ignore...",
                LOG_FILE_PATH(__FILE__), __LINE__,
                fileName);
            continue;
        }

        name = name.substr(0, index);
        if (createQueue(name))
        {
            spdlog::error("{}:{} Fail to create queue: {}", LOG_FILE_PATH(__FILE__), __LINE__,
                          name);
            m_conn = nullptr;
            m_queueList.clear();
            return ErrCode_OS_ERROR;
        }
    }

    return ErrCode_OK;
}

u8 SQLiteQueueList::createQueue(const std::string &name)
{
    spdlog::debug("{}:{} SQLiteQueueList::createQueue", LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("{}:{} name: {}", LOG_FILE_PATH(__FILE__), __LINE__, name.c_str());

    if (m_queueList.find(name) != m_queueList.end())
    {
        spdlog::error("{}:{} {} is already exists", LOG_FILE_PATH(__FILE__), __LINE__, name);
        return ErrCode_ALREADY_EXISTS;
    }

#ifdef _WIN32
    Proc::WinProc *proc = new (std::nothrow) Proc::WinProc();
#elif defined(__linux__)
    Proc::LinuxProc *proc = new (std::nothrow) Proc::LinuxProc();
#else
    Proc::MacProc *proc = new (std::nothrow) Proc::MacProc();
#endif
    if (!proc)
    {
        spdlog::error("{}:{} Fail to allocate memory", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_OS_ERROR;
    }

    if (proc->init())
    {
        delete proc;
        spdlog::error("{}:{} Fail to initialize process", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_OS_ERROR;
    }

    SQLiteQueue *queue = new (std::nothrow) SQLiteQueue();
    if (!queue)
    {
        delete proc;
        spdlog::error("{}:{} Fail to allocate memory", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_OS_ERROR;
    }

    if (queue->init(m_conn, proc, name))
    {
        delete queue;
        spdlog::error("{}:{} Fail to initialize queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_OS_ERROR;
    }

    m_queueList[name] = std::shared_ptr<IQueue>(queue);
    return ErrCode_OK;
}

u8 SQLiteQueueList::listQueue(std::vector<std::string> &out)
{
    spdlog::debug("{}:{} SQLiteQueueList::listQueue", LOG_FILE_PATH(__FILE__), __LINE__);

    out.clear();
    out.reserve(m_queueList.size());
    for (auto it = m_queueList.begin();
         it != m_queueList.end();
         ++it)
    {
        out.push_back(it->first);
    }

    return ErrCode_OK;
}

u8 SQLiteQueueList::deleteQueue(const std::string &name)
{
    spdlog::debug("{}:{} SQLiteQueueList::deleteQueue", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!m_queueList.erase(name))
    {
        spdlog::error("{}:{} No such queue: {}", LOG_FILE_PATH(__FILE__), __LINE__,
            name);
        return ErrCode_NOT_FOUND;
    }

    std::remove((m_conn->targetPath() + "/" + name + ".db").c_str());
    return ErrCode_OK;
}

u8
SQLiteQueueList::renameQueue(const std::string &oldName,
                             const std::string &newName)
{
    spdlog::debug("{}:{} SQLiteQueueList::renameQueue", LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("{}:{} oldName: {}", LOG_FILE_PATH(__FILE__), __LINE__, oldName.c_str());
    spdlog::debug("{}:{} newName: {}", LOG_FILE_PATH(__FILE__), __LINE__, newName.c_str());

    for (auto &it : m_queueList)
    {
        if (it.first == oldName)
        {
            SQLiteQueue *queue = static_cast<SQLiteQueue *>(it.second.get());
            if (queue->rename(newName, oldName))
            {
                spdlog::error("{}:{} Fail to rename", LOG_FILE_PATH(__FILE__), __LINE__);
                return ErrCode_OS_ERROR;
            }

            m_queueList[newName] = it.second;
            m_queueList.erase(oldName);
            return ErrCode_OK;
        }
    }

    spdlog::error("{}:{} No such queue: {}", LOG_FILE_PATH(__FILE__), __LINE__,
        oldName);
    return ErrCode_NOT_FOUND;
}

IQueue *SQLiteQueueList::getQueue(const std::string &name)
{
    spdlog::debug("{}:{} SQLiteQueueList::getQueue", LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("{}:{} name: {}", LOG_FILE_PATH(__FILE__), __LINE__, name.c_str());

    auto it = m_queueList.find(name);
    if (it == m_queueList.end()) return nullptr;
    return it->second.get();
}

void SQLiteQueueList::returnQueue(IQueue *)
{
    spdlog::debug("{}:{} SQLiteQueueList::returnQueue", LOG_FILE_PATH(__FILE__), __LINE__);
}

} // end namespace DAO

} // end namespace Model
