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

#include <iostream>
#include <new>

#include "model/dao/grpcconnect.hpp"
#include "model/dao/grpcqueuelist.hpp"

#include "param.hpp"

namespace Model
{

namespace Test
{

Param::~Param()
{
    cleanUp();
}

u8 Param::setupGRPC()
{
#ifdef _WIN32
    m_proc = new (std::nothrow) Proc::WinProc();
#else
    m_proc = new (std::nothrow) Proc::LinuxProc();
#endif

    if (!m_proc)
    {
        printLog(__FILE__, __LINE__, "Fail to allocate memory");
        cleanUp();
        return 1;
    }

    if (m_proc->init())
    {
        printLog(__FILE__, __LINE__, "proc init failed");
        cleanUp();
        return 1;
    }

    Model::Proc::Task task;

    task.execName = "FlexFlowServer";
    task.workDir = ".";

    if (m_proc->start(task))
    {
        printLog(__FILE__, __LINE__, "Fail to start Flex Flow server");
        cleanUp();
        return 1;
    }

    DAO::GRPCConnect *conn = new (std::nothrow) DAO::GRPCConnect;
    if (!conn)
    {
        printLog(__FILE__, __LINE__, "Fail to allocate memory");
        cleanUp();
        return 1;
    }

    if (conn->init())
    {
        printLog(__FILE__, __LINE__, "Conn init failed");
        cleanUp();
        delete conn;
        return 1;
    }

    if (conn->startConnect("127.0.0.1", 12345))
    {
        printLog(__FILE__, __LINE__, "Fail to connect to Flex Flow server");
        cleanUp();
        delete conn;
        return 1;
    }

    auto list = new (std::nothrow) DAO::GRPCQueueList;
    if (!list)
    {
        printLog(__FILE__, __LINE__, "Fail to allocate memory");
        cleanUp();
        delete conn;
        return 1;
    }

    m_list = list;
    if (m_list->init(conn))
    {
        printLog(__FILE__, __LINE__, "Fail to allocate memory");
        cleanUp();
        delete conn;
        return 1;
    }

    return 0;
}

u8 Param::mainTest()
{
    return 0;
}

// private member functions
void Param::printLog(const char *file, int line, const char *log)
{
    if (!file || !log)
    {
        return;
    }

    std::cout << file << ":" << line << " " << log << std::endl;
}

void Param::cleanUp()
{
    if (m_proc)
    {
        delete m_proc;
        m_proc = nullptr;
    }

    if (m_list)
    {
        delete m_list;
        m_list = nullptr;
    }
}

} // namespace Test

} // namespace Model
