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

#include <iostream>

#include <csignal>

#include "controller/cppcli/config.hpp"
#include "spdlog/spdlog.h"


#ifdef _WIN32
#include "windows.h"
#endif

#include "controller/global/global.hpp"
#include "model/utils.hpp"
#include "global.hpp"
#include "queuelist.hpp"

#include "main.hpp"

namespace Controller
{

namespace CppCLI
{

static void sighandler(int signum);

#ifdef _WIN32
static BOOL eventHandler(DWORD dwCtrlType);
#endif

Main::Main()
{}

Main::~Main()
{
    Controller::Global::consoleFin();
}

// public member functions
i32 Main::init(int argc, char **argv)
{
    if (!argv)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "you should never see this line");
        return 1;
    }

    if (Controller::Global::isAdmin())
    {
#ifdef _WIN32
        spdlog::error("{}:{} Refuse to run as administrator", __FILE__, __LINE__);
#else
        spdlog::error("{}:{} Refuse to run as super user", __FILE__, __LINE__);
#endif
        return 1;
    }

    i32 ret(parseArgs(argc, argv));
    if (ret)
    {
        spdlog::error("{}:{} Fail to parse args.", __FILE__, __LINE__);
        return ret;
    }

    if (Controller::Global::spdlogInit(Global::config.logFile))
    {
        spdlog::error("{}:{} spdlog init failed", __FILE__, __LINE__);
        return 1;
    }

    if (optsInit())
    {
        spdlog::error("{}:{} Opts init failed", __FILE__, __LINE__);
        return 1;
    }

    spdlog::set_level(static_cast<spdlog::level::level_enum>(Global::config.logLevel));
    Global::keepRunning.store(true, std::memory_order_relaxed);
    if (Global::args.init())
    {
        spdlog::error("{}:{} Args init failed", __FILE__, __LINE__);
        return 1;
    }

    signal(SIGABRT, sighandler);
    signal(SIGFPE,  sighandler);
    signal(SIGILL,  sighandler);
    signal(SIGINT,  sighandler);
    signal(SIGSEGV, sighandler);
    signal(SIGTERM, sighandler);

#ifdef _WIN32
    SetConsoleCtrlHandler(eventHandler, TRUE);
#endif

    return 0;
}

i32 Main::run()
{
    i32 ret(0);
    std::string prefix = "> ";

    if (Global::config.autoConnect)
    {
        ret = connect();
    }

    while (Global::keepRunning.load(std::memory_order_relaxed))
    {
        if (Global::args.getArgs(prefix))
        {
            fmt::println("Fail to get command");
            continue;
        }

        char **argv = Global::args.argv();

        // only accept "print" "modify" "connect" "exit"
        if (!memcmp(argv[0], "print", 5))
        {
            ret = print();
            continue;
        }

        if (!memcmp(argv[0], "modify", 7))
        {
            ret = modify();
            continue;
        }

        if (!memcmp(argv[0], "connect", 8))
        {
            ret = connect();
            continue;
        }

        if (!memcmp(argv[0], "exit", 4))
        {
            Global::keepRunning.store(false, std::memory_order_relaxed);
            fmt::println("Good bye!");
            continue;
        }

        if (!memcmp(argv[0], "help", 4))
        {
            fmt::println("Valid command: print, modify, connect, help, exit");
            fmt::println("Please type \"<command> -h\" for more details.");
            fmt::println("Please type \"help\" to show this message.");
            fmt::println("Please type \"exit\" to exit.");
            ret = 0;
            continue;
        }

        fmt::println("Invalid command: {}", Global::args.args().at(0));
        fmt::println("Please type \"help\" for more info");
        ret = 1;
    }

    return ret;
}

// private menber functions
i32 Main::parseArgs(int argc, char **argv)
{
    if (!argv)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "you should never see this line");
        return 1;
    }

    try
    {
        cxxopts::Options options("FFCLI", "FF CLI Client");
        options.add_options()
            ("l,log-file", "file for output log", cxxopts::value<std::string>(Global::config.logFile)->default_value(""))
            ("L,log-level", "log level for spdlog", cxxopts::value<i32>(Global::config.logLevel)->default_value("2"))
            ("a,address", "address to FF server", cxxopts::value<std::string>(Global::config.address)->default_value("127.0.0.1"))
            ("A,auto-connect", "auto connect to server", cxxopts::value<bool>(Global::config.autoConnect)->default_value("true"))
            ("p,port", "which port is FF Server listening", cxxopts::value<u16>(Global::config.port)->default_value("12345"))
            ("v,version", "print version")
            ("h,help", "print help")
            ;

        auto result = options.parse(argc, argv);
        if (result.count("help"))
        {
            fmt::print("{}", options.help());
            return 2;
        }

        if (result.count("version"))
        {
            printVersion();
            return 2;
        }
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::print("{}", e.what());
        return 1;
    }

    return 0;
}

void Main::printVersion()
{
    fmt::println("FFCLI version info:");
    fmt::println("branch: " FF_BRANCH);
    fmt::println("commit: " FF_COMMIT);
    fmt::println("version: " FF_VERSION);
}

u8 Main::optsInit()
{
    try
    {
        // print
        printOpts.add_options()
            ("i,info", "print info")
            ("h,help", "print help");

        // modify
        modifyOpts.add_options()
            ("b,backend", "modify backend, 0 = grpc, 1 = sqlite", cxxopts::value<u8>(Global::config.backend))
            ("a,address", "modify server address", cxxopts::value<std::string>(Global::config.address))
            ("p,port", "modify server port", cxxopts::value<u16>(Global::config.port))
            ("h,help", "print help");
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    return 0;
}

i32 Main::print()
{
    if (Global::args.argc() == 1)
    {
        fmt::print("{}", printOpts.help());
        return 0;
    }

    try
    {
        auto result = printOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::print("{}", printOpts.help());
            return 0;
        }

        if (result.count("info"))
        {
            printVersion();

            fmt::println("");
            fmt::println("target host: {}", Global::config.address);
            fmt::println("target port: {}", Global::config.port);
        }
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    return 0;
}

i32 Main::modify()
{
    if (Global::args.argc() == 1)
    {
        fmt::println("{}", modifyOpts.help());
        return 0;
    }

    try
    {
        auto result = modifyOpts.parse(Global::args.argc(), Global::args.argv());
        if (result.count("help"))
        {
            fmt::println("{}", modifyOpts.help());
            return 0;
        }

        if (Global::config.backend > BACKEND_SQLITE)
        {
            spdlog::warn("{}:{} Invaild backend, set backend to \"sqlite\"", __FILE__, __LINE__);
            Global::config.backend = BACKEND_SQLITE;
        }

        fmt::println("done");
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        fmt::println("{}", e.what());
        return 1;
    }

    return 0;
}

i32 Main::connect()
{
    if (Global::args.argc() > 1)
    {
        fmt::println("Invalid command");
        return 1;
    }

    QueueList queueList;
    if (queueList.init())
    {
        fmt::println("Fail to connect to server");
        return 1;
    }

    return queueList.run();
}

static void sighandler(int signum)
{
    spdlog::info("{}:{} Signaled: {}", __FILE__, __LINE__, signum);
    spdlog::info("{}:{} Good Bye!", __FILE__, __LINE__);
    Global::keepRunning.store(false, std::memory_order_relaxed);
    exit(0);
}

#ifdef _WIN32
static BOOL eventHandler(DWORD dwCtrlType)
{
    sighandler(dwCtrlType);
    return TRUE;
}
#endif

} // end namespace CppCLI

} // end namespace Controller
