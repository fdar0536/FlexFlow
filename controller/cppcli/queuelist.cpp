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

#include "spdlog/spdlog.h"

#include "controller/global/global.hpp"
#include "controller/cppcli/config.hpp"
#include "controller/cppcli/global.hpp"

#include "queue.hpp"
#include "queuelist.hpp"

namespace Controller
{

namespace CppCLI
{

QueueList::QueueList() :
    m_queueList(nullptr)
{}

QueueList::~QueueList()
{
    if (m_queueList) delete m_queueList;
}

// public member functions
u8 QueueList::init()
{
    u8 ret(0);
    switch (Global::config.backend)
    {
        case BACKEND_GRPC:
        {
            ret = Controller::Global::grpcInit(m_queueList,
                                               Global::config.address,
                                               Global::config.port);
            break;
        }
        case BACKEND_SQLITE:
        {
            ret = Controller::Global::sqliteInit(m_queueList,
                                                 Global::config.address);
            break;
        }
        default:
        {
            spdlog::error("{}:{} Invaild backend.", __FILE__, __LINE__);
            return 1;
        }
    }

    if (ret)
    {
        spdlog::error("{}:{} Init failed.", __FILE__, __LINE__);
        return 1;
    }

    m_funcs["create"] = std::bind(&QueueList::create, this);
    m_funcs["delete"] = std::bind(&QueueList::Delete, this);
    m_funcs["list"] = std::bind(&QueueList::list, this);
    m_funcs["rename"] = std::bind(&QueueList::rename, this);

    m_createOpts.add_options()
        ("n,name", "the new queue name, the name cannot be empty", cxxopts::value<std::string>())
        ("h,help", "print help");

    m_deleteOpts.add_options()
        ("n,name", "the new queue name, the name cannot be empty", cxxopts::value<std::string>())
        ("h,help", "print help");

    m_listOpts.add_options()
        ("h,help", "print help");

    m_renameOpts.add_options()
        ("s,source", "the queue to rename, cannot be empty", cxxopts::value<std::string>())
        ("t,target", "the new name for source, cannot be empty", cxxopts::value<std::string>())
        ("h,help", "print help");

    return 0;
}

i32 QueueList::run()
{
    m_prefix = Global::config.address + ":";
    m_prefix += std::to_string(Global::config.port);
    std::string prefix = m_prefix + "> ";
    i32 ret(0);

    while (Global::keepRunning.load(std::memory_order_relaxed))
    {
        // vaild command: create delete list rename help exit
        if (Global::args.getArgs(prefix))
        {
            fmt::println("Fail to get command");
            continue;
        }

        if (Global::args.argc() == 1)
        {
            // vaild command: help exit "queue name"
            if (Global::args.args().at(0) == "help")
            {
                fmt::println("Valid command: create delete list rename help exit");
                fmt::println("Please type \"<command> -h\" for more details.");
                fmt::println("Please type \"help\" to show this message.");
                fmt::println("Please type \"exit\" to exit.");
                fmt::println("Or just type queue name which you want to enter");

                ret = 0;
                continue;
            }

            if (Global::args.args().at(0) == "exit")
            {
                return ret;
            }
        }

        try
        {
            ret = m_funcs[Global::args.args().at(0)]();
        }
        catch(...)
        {
            // cannot interpret
            ret = enter();
            continue;
        }
    }

    return ret;
}

// private member functions
i32 QueueList::create()
{
    if (Global::args.argc() == 1)
    {
        fmt::print("{}", m_createOpts.help());
        return 0;
    }

    std::string name;
    try
    {
        auto result = m_createOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_createOpts.help());
            return 0;
        }

        if (!result.count("name"))
        {
            fmt::print("{}", m_createOpts.help());
            return 1;
        }

        name = result["name"].as<std::string>();
        if (name.empty())
        {
            fmt::print("{}", m_createOpts.help());
            return 1;
        }
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    if (m_queueList->createQueue(name))
    {
        fmt::println("Fail to create queue");
        return 1;
    }

    fmt::println("done");
    return 0;
}

i32 QueueList::Delete()
{
    if (Global::args.argc() == 1)
    {
        fmt::print("{}", m_deleteOpts.help());
        return 0;
    }

    std::string name;
    try
    {
        auto result = m_deleteOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_deleteOpts.help());
            return 0;
        }

        if (!result.count("name"))
        {
            fmt::print("{}", m_deleteOpts.help());
            return 1;
        }

        name = result["name"].as<std::string>();
        if (name.empty())
        {
            fmt::print("{}", m_deleteOpts.help());
            return 1;
        }
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    if (m_queueList->deleteQueue(name))
    {
        fmt::println("Fail to delete queue");
        return 1;
    }

    fmt::println("done");
    return 0;
}

i32 QueueList::list()
{
    if (Global::args.argc() > 1)
    {
        fmt::print("{}", m_listOpts.help());
        return 1;
    }

    try
    {
        auto result = m_listOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_listOpts.help());
            return 0;
        }
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    std::vector<std::string> out;
    if (m_queueList->listQueue(out))
    {
        fmt::println("Fail to list queue");
        return 1;
    }

    fmt::println("");
    for (size_t i = 0; i < out.size(); ++i)
    {
        fmt::println("{}", out.at(i));
    }

    return 0;
}

i32 QueueList::rename()
{
    if (Global::args.argc() == 1)
    {
        fmt::print("{}", m_renameOpts.help());
        return 0;
    }

    std::string source, target;
    try
    {
        auto result = m_renameOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", m_renameOpts.help());
            return 0;
        }

        if (!result.count("source"))
        {
            fmt::print("{}", m_renameOpts.help());
            return 1;
        }

        source = result["source"].as<std::string>();
        if (source.empty())
        {
            fmt::print("{}", m_renameOpts.help());
            return 1;
        }

        if (!result.count("target"))
        {
            fmt::print("{}", m_renameOpts.help());
            return 1;
        }

        target = result["target"].as<std::string>();
        if (target.empty())
        {
            fmt::print("{}", m_renameOpts.help());
            return 1;
        }
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    if (m_queueList->renameQueue(source, target))
    {
        fmt::println("Fail to rename queue");
        return 1;
    }

    fmt::println("done");
    return 0;
}

i32 QueueList::enter()
{
    auto ptr = m_queueList->getQueue(Global::args.args().at(0));
    if (!ptr)
    {
        fmt::println("Fail to enter the queue");
        return 1;
    }

    Queue queue;
    if (queue.init())
    {
        fmt::println("Fail to enter the queue");
        return 1;
    }

    i32 ret = queue.run(m_prefix + "/" + Global::args.args().at(0) + "> ", ptr);
    m_queueList->returnQueue(ptr);
    return ret;
}

} // end namespace Cpp

} // end namespace Controller
