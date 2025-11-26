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

#ifdef _WIN32
#include "winsock.h"
#include "direct.h"
#include "conio.h"
#define sleep(x) Sleep(x * 1000)
#else
#include <stdio.h>
#include "sys/ioctl.h"
#include "termios.h"
#include "unistd.h"
#include "time.h"
#endif

#include "spdlog/spdlog.h"

#include "model/utils.hpp"

#include "controller/cppcli/global.hpp"

#include "queue.hpp"

namespace Controller
{

namespace CppCLI
{

Queue::~Queue()
{
    if (m_queue) delete m_queue;
}

u8 Queue::init()
{
    m_funcs["list"] = std::bind(&Queue::list, this);
    m_funcs["details"] = std::bind(&Queue::details, this);
    m_funcs["clear"] = std::bind(&Queue::clear, this);
    m_funcs["remove"] = std::bind(&Queue::remove, this);
    m_funcs["current"] = std::bind(&Queue::current, this);
    m_funcs["add"] = std::bind(&Queue::add, this);
    m_funcs["isRunning"] = std::bind(&Queue::isRunning, this);
    m_funcs["start"] = std::bind(&Queue::start, this);
    m_funcs["stop"] = std::bind(&Queue::stop, this);
    m_funcs["output"] = std::bind(&Queue::output, this);

    m_listOpts.add_options()
        ("m,mode", "list task, 1 for pending, 2 for finished", cxxopts::value<u8>()->default_value("1"))
        ("h,help", "print help");

    m_detailsOpts.add_options()
        ("m,mode", "for which list, 1 for pending, 2 for finished", cxxopts::value<u8>()->default_value("1"))
        ("i,id", "the task id", cxxopts::value<i32>()->default_value("0"))
        ("h,help", "print help");

    m_clearOpts.add_options()
        ("m,mode", "list task, 1 for pending, 2 for finished", cxxopts::value<u8>()->default_value("1"))
        ("h,help", "print help");

    m_currentOpts.add_options()
        ("h,help", "print help");

    char buf[1024];
    size_t len(1024);
#ifdef _WIN32
    if (!_getcwd(buf, len))
#else
    if (!getcwd(buf, len))
#endif
    {
        spdlog::error("{}:{} Fail to get current path", __FILE__, __LINE__);
        return 1;
    }

    m_addOpts.add_options()
        ("w,workDir", "set working dir", cxxopts::value<std::string>()->default_value(std::string(buf)))
        ("e,exec", "program to execute", cxxopts::value<std::string>())
        ("a,args", "command line arguments for the program to execute", cxxopts::value<std::vector<std::string>>())
        ("h,help", "print help");

    m_removeOpts.add_options()
        ("i,id", "id for task to remove", cxxopts::value<std::vector<i32>>())
        ("h,help", "print help");

    m_isRunningOpts.add_options()
        ("h,help", "print help");

    m_startOpts.add_options()
        ("h,help", "print help");

    m_stopOpts.add_options()
        ("h,help", "print help");

    m_outputOpts.add_options()
        ("h,help", "print help");

    return 0;
}

i32 Queue::run(const std::string &prefix,
               Model::DAO::IQueue *queuePtr)
{
    if (!queuePtr)
    {
        spdlog::error("{}:{} you should never see this line");
        fmt::println("queuePtr is nullptr");
        return 1;
    }

    m_queue = queuePtr;

    i32 ret(0);
    while (Global::keepRunning.load(std::memory_order_relaxed))
    {
        if (Global::args.getArgs(prefix))
        {
            fmt::println("Fail to get command");
            continue;
        }

        if (Global::args.argc() == 1)
        {
            // vaild commands: help exit
            if (Global::args.args().at(0) == "help")
            {
                fmt::print("Vaild commands: list details clear ");
                fmt::print("remove current add isRunning ");
                fmt::println("start stop output help exit");
                fmt::println("Please type \"<command> -h\" for more details.");
                fmt::println("Please type \"help\" to show this message.");
                fmt::println("Please type \"exit\" to exit.");

                ret = 0;
                continue;
            }

            if (Global::args.args().at(0) == "exit")
            {
                break;
            }
        }

        try
        {
            ret = m_funcs[Global::args.args().at(0)]();
        }
        catch(...)
        {
            fmt::println("Invalid command: {}", Global::args.args().at(0));
            fmt::println("Please type \"help\" for more info");
            ret = 1;
            continue;
        }
    }

    return ret;
}

i32 Queue::printList(u8 exitCode, const std::string &name, const std::vector<int> &out)
{
    if (exitCode)
    {
        fmt::println("Fail to list tasks");
        return 1;
    }

    fmt::println("");
    if (out.empty())
    {
        fmt::println("{} list is empty", name);
        return 0;
    }

    for (auto i = out.begin(); i != out.end(); ++i)
    {
        fmt::println("{}", *i);
    }

    return 0;
}

#define PENDING  1
#define FINISHED 2

i32 Queue::list()
{
    if (Global::args.argc() == 1)
    {
        fmt::print("{}", m_listOpts.help());
        return 0;
    }

    u8 ret(0);
    std::vector<int> out;

    try
    {
        auto result = m_listOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_listOpts.help());
            return 0;
        }

        u8 mode = result["mode"].as<u8>();
        switch(mode)
        {
        case PENDING:
        {
            fmt::println("listing pending list...");
            ret = m_queue->listPending(out);
            return printList(ret, "pending", out);
        }
        case FINISHED:
        {
            fmt::println("listing finished list...");
            ret = m_queue->listFinished(out);
            return printList(ret, "finished", out);
        }
        default:
        {
            fmt::print("{}", m_listOpts.help());
            return 1;
        }
        };
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
    }

    return 1;
}

i32 Queue::details()
{
    if (Global::args.argc() == 1)
    {
        fmt::print("{}", m_detailsOpts.help());
        return 0;
    }

    u8 ret(0);
    Model::Proc::Task out;
    try
    {
        auto result = m_detailsOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_detailsOpts.help());
            return 0;
        }

        u8 mode = result["mode"].as<u8>();
        i32 id = result["id"].as<i32>();
        switch (mode)
        {
        case PENDING:
        {
            fmt::println("pending task details...");
            ret = m_queue->pendingDetails(id, out);
            break;
        }
        case FINISHED:
        {
            fmt::println("finished task details...");
            ret = m_queue->finishedDetails(id, out);
            break;
        }
        default:
        {
            fmt::print("{}", m_detailsOpts.help());
            return 1;
        }
        };
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    if (ret)
    {
        fmt::println("Fail to get task details");
        return 1;
    }

    out.print();
    return 0;
}

i32 Queue::clear()
{
    if (Global::args.argc() == 1)
    {
        fmt::print("{}", m_clearOpts.help());
        return 0;
    }

    u8 ret(0);
    try
    {
        auto result = m_clearOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_clearOpts.help());
            return 0;
        }

        u8 mode = result["mode"].as<u8>();
        switch (mode)
        {
        case PENDING:
        {
            ret = m_queue->clearPending();
            break;
        }
        case FINISHED:
        {
            ret = m_queue->clearFinished();
            break;
        }
        default:
        {
            fmt::print("{}", m_clearOpts.help());
            return 1;
        }
        };
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    if (ret)
    {
        fmt::println("Fail to clear tasks");
        return 1;
    }

    fmt::println("done");
    return 0;
}

#undef PENDING
#undef FINISHED

i32 Queue::current()
{
    if (Global::args.argc() > 2)
    {
        fmt::print("{}", m_currentOpts.help());
        return 1;
    }

    try
    {
        auto result = m_currentOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_currentOpts.help());
            return 0;
        }
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    Model::Proc::Task out;
    if (m_queue->currentTask(out))
    {
        fmt::println("Fail to get current task");
        return 1;
    }

    out.print();
    return 0;
}

i32 Queue::add()
{
    if (Global::args.argc() == 1)
    {
        fmt::print("{}", m_addOpts.help());
        return 0;
    }

    Model::Proc::Task in;
    try
    {
        auto result = m_addOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_addOpts.help());
            return 0;
        }

        if (result.count("exec") < 1)
        {
            fmt::println("No file to execute");
            return 1;
        }

        in.workDir = result["workDir"].as<std::string>();
        in.execName = result["exec"].as<std::string>();
        if (result.count("args"))
        {
            std::vector<std::string> args = result["args"].as<std::vector<std::string>>();
            in.args.insert(in.args.end(),
                           args.begin(),
                           args.end());
        }
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    if (m_queue->addTask(in))
    {
        fmt::println("Fail to add task");
        return 1;
    }

    fmt::println("done");
    return 0;
}

i32 Queue::remove()
{
    if (Global::args.argc() == 1)
    {
        fmt::print("{}", m_removeOpts.help());
        return 0;
    }

    std::vector<i32> id;
    try
    {
        auto result = m_removeOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_removeOpts.help());
            return 0;
        }

        if (!result.count("id"))
        {
            fmt::print("{}", m_removeOpts.help());
            return 1;
        }

        id = result["id"].as<std::vector<i32>>();
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    for (auto it = id.begin(); it != id.end(); ++it)
    {
        if (m_queue->removeTask(*it))
        {
            fmt::println("Fail to remove task: {}", *it);
        }
    }

    fmt::println("done");
    return 0;
}

i32 Queue::isRunning()
{
    if (Global::args.argc() > 2)
    {
        fmt::print("{}", m_isRunningOpts.help());
        return 1;
    }

    try
    {
        auto result = m_isRunningOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_isRunningOpts.help());
            return 0;
        }
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    if (m_queue->isRunning())
    {
        fmt::println("ture");
    }
    else
    {
        fmt::println("false");
    }

    return 0;
}

i32 Queue::start()
{
    if (Global::args.argc() > 2)
    {
        fmt::print("{}", m_startOpts.help());
        return 1;
    }

    try
    {
        auto result = m_startOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_startOpts.help());
            return 0;
        }
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    if (m_queue->start())
    {
        fmt::println("Fail to start the queue");
        return 1;
    }

    fmt::println("done");
    return 0;
}

i32 Queue::stop()
{
    if (Global::args.argc() > 2)
    {
        fmt::print("{}", m_stopOpts.help());
        return 1;
    }

    try
    {
        auto result = m_stopOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_stopOpts.help());
            return 0;
        }
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    m_queue->stop();
    fmt::println("done");
    return 0;
}

i32 Queue::output()
{
    if (Global::args.argc() > 2)
    {
        fmt::print("{}", m_outputOpts.help());
        return 1;
    }

    try
    {
        auto result = m_outputOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_outputOpts.help());
            return 0;
        }
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    std::vector<std::string> out;
    m_queue->readCurrentOutput(out);
    if (!out.size())
    {
        // out.size() == 0
        // read nothing
        return 0;
    }

    for (auto it = out.begin();
         it != out.end();
         ++it)
    {
        fmt::println("{}", *it);
    }

    return 0;
}

} // end namespace CppCLI

} // end namespace Controller
