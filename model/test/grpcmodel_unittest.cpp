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

#include "atomic"
#include "thread"

#include "gtest/gtest.h"

#include "model/dao/grpcconnect.hpp"
#include "model/dao/grpcqueuelist.hpp"

#include "param.hpp"

class GRPCModelTesting : public Model::Test::Param
{
public:

    ~GRPCModelTesting()
    {
        stop();
    }

    u8 setupConn() override
    {
#ifdef _WIN32
        m_proc = new (std::nothrow) Model::Proc::WinProc();
#else
        m_proc = new (std::nothrow) Model::Proc::LinuxProc();
#endif

        if (!m_proc)
        {
            printLog(__FILE__, __LINE__, "Fail to allocate memory");
            stop();
            return 1;
        }

        if (m_proc->init())
        {
            printLog(__FILE__, __LINE__, "proc init failed");
            stop();
            return 1;
        }

        Model::Proc::Task task;

        task.execName = "FlexFlowServer";
        task.workDir = ".";

        if (m_proc->start(task))
        {
            printLog(__FILE__, __LINE__, "Fail to start Flex Flow server");
            stop();
            return 1;
        }

        m_keepRunning.store(true, std::memory_order_relaxed);
        m_thread = std::jthread(&GRPCModelTesting::readProcOutput, this);

        Model::DAO::GRPCConnect *conn = new (std::nothrow) Model::DAO::GRPCConnect;
        if (!conn)
        {
            printLog(__FILE__, __LINE__, "Fail to allocate memory");
            stop();
            return 1;
        }

        if (conn->init())
        {
            printLog(__FILE__, __LINE__, "Conn init failed");
            stop();
            delete conn;
            return 1;
        }

        if (conn->startConnect("127.0.0.1", 12345))
        {
            printLog(__FILE__, __LINE__, "Fail to connect to Flex Flow server");
            stop();
            delete conn;
            return 1;
        }

        auto list = new (std::nothrow) Model::DAO::GRPCQueueList;
        if (!list)
        {
            printLog(__FILE__, __LINE__, "Fail to allocate memory");
            stop();
            delete conn;
            return 1;
        }

        m_list = list;
        if (m_list->init(conn))
        {
            printLog(__FILE__, __LINE__, "Fail to allocate memory");
            stop();
            delete conn;
            return 1;
        }

        return 0;
    }

private:

    std::atomic<bool> m_keepRunning;

    std::jthread m_thread;

    void readProcOutput()
    {
        std::vector<std::string> out;
        while(m_keepRunning.load(std::memory_order_relaxed))
        {
            m_proc->readCurrentOutput(out);
            for (size_t i = 0; i < out.size(); ++i)
            {
                std::cout << out.at(i);
            }
        }
    }

    void stop()
    {
        m_keepRunning.store(false, std::memory_order_relaxed);
        cleanUp();
    }
};

TEST(GRPCModel, Testing)
{
    GRPCModelTesting param;
    EXPECT_EQ(param.setupConn(), 0);

    EXPECT_EQ(param.createQueue("Test1"), 0);
    EXPECT_EQ(param.createQueue("Test1"), 1);
    EXPECT_EQ(param.createQueue("Test2"), 0);
}
