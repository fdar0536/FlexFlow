/*
 * Simple Task Queue
 * Copyright (c) 2023-2024 fdar0536
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

#include "model/errmsg.hpp"

#include "sqlitequeue.hpp"
#include "sqlitequeuelist.hpp"
#include "dirutils.hpp"

#ifdef _WIN32
#include "model/proc/winproc.hpp"
#else
#include "model/proc/posixproc.hpp"
#endif

namespace Model
{

namespace DAO
{

SQLiteQueueList::SQLiteQueueList()
{}

SQLiteQueueList::~SQLiteQueueList()
{}

u8
SQLiteQueueList::init(std::shared_ptr<IConnect> &connect)
{
    if (connect == nullptr)
    {
        spdlog::error("{}:{} \"connect\" is nullptr.", __FILE__, __LINE__);
        return ErrCode_INVALID_ARGUMENT;
    }

    if (DirUtils::verifyDir(connect->targetPath()))
    {
        spdlog::error("{}:{} Fail to verify target path.", __FILE__, __LINE__);
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
            spdlog::warn("{}:{} {} is directory, ignore...", __FILE__, __LINE__,
                         entry.path().string());
            continue;
        }

        if (!std::filesystem::is_regular_file(entry))
        {
            spdlog::warn("{}:{} {} is not regular file, ignore...", __FILE__, __LINE__,
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
            spdlog::warn("{}:{} {} is not database, ignore...", __FILE__, __LINE__,
                fileName);
            continue;
        }

        name = name.substr(0, index);
        if (createQueue(name))
        {
            spdlog::error("{}:{} Fail to create queue: {}", __FILE__, __LINE__,
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
#ifdef _WIN32
    Proc::WinProc *proc = new (std::nothrow) Proc::WinProc();
#else
    Proc::PosixProc *proc = new (std::nothrow) Proc::PosixProc();
#endif
    if (!proc)
    {
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return ErrCode_OS_ERROR;
    }

    if (proc->init())
    {
        delete proc;
        spdlog::error("{}:{} Fail to initialize process", __FILE__, __LINE__);
        return ErrCode_OS_ERROR;
    }

    SQLiteQueue *queue = new (std::nothrow) SQLiteQueue();
    if (!queue)
    {
        delete proc;
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return ErrCode_OS_ERROR;
    }

    std::shared_ptr<Proc::IProc> procPtr = std::shared_ptr<Proc::IProc>(proc);
    if (queue->init(m_conn, procPtr, name))
    {
        delete queue;
        spdlog::error("{}:{} Fail to initialize queue", __FILE__, __LINE__);
        return ErrCode_OS_ERROR;
    }

    m_queueList[name] = std::shared_ptr<IQueue>(queue);
    return ErrCode_OK;
}

u8 SQLiteQueueList::listQueue(std::vector<std::string> &out)
{
    out.clear();
    out.reserve(m_queueList.size());
    for (auto it = m_queueList.begin();
         it != m_queueList.end();
         ++it)
    {
        out.push_back(it->first);
    }

    if (!out.size())
    {
        spdlog::error("{}:{} queue list is empty", __FILE__, __LINE__);
        return ErrCode_NOT_FOUND;
    }

    return ErrCode_OK;
}

u8 SQLiteQueueList::deleteQueue(const std::string &name)
{
    if (!m_queueList.erase(name))
    {
        spdlog::error("{}:{} No such queue: {}", __FILE__, __LINE__,
            name);
        return ErrCode_NOT_FOUND;
    }

    return ErrCode_OK;
}

u8
SQLiteQueueList::renameQueue(const std::string &oldName,
                             const std::string &newName)
{
    for (auto &it : m_queueList)
    {
        if (it.first == oldName)
        {
            m_queueList[newName] = it.second;
            m_queueList.erase(oldName);
            return ErrCode_OK;
        }
    }

    spdlog::error("{}:{} No such queue: {}", __FILE__, __LINE__,
        oldName);
    return ErrCode_NOT_FOUND;
}

std::shared_ptr<IQueue>
SQLiteQueueList::getQueue(const std::string &name)
{
    auto it = m_queueList.find(name);
    if (it == m_queueList.end()) return nullptr;
    return it->second;

}

} // end namespace DAO

} // end namespace Model
