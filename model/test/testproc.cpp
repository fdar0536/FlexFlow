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

#include <iostream>
#include <csignal>

#include "cxxopts.hpp"

#ifdef _WIN32
#include "windows.h"
#define sleep(x) Sleep(x * 1000)
#else
#include "unistd.h"
#endif

#include "controller/global/defines.h"
#include "controller/global/global.hpp"

typedef struct Params
{
    int exitCode = 0;
    i32 loopTimes = 5;
    int sleepTime = 1;
} Params;

static Params params;

static void loopLimited();

static void loopForever();

static void sighandler(int signum);

#ifdef _WIN32
static BOOL eventHandler(DWORD dwCtrlType);
#endif

int main(int argc, char **argv)
{
    signal(SIGABRT, sighandler);
    signal(SIGFPE,  sighandler);
    signal(SIGILL,  sighandler);
    signal(SIGINT,  sighandler);
    signal(SIGSEGV, sighandler);
    signal(SIGTERM, sighandler);

#ifdef _WIN32
    SetConsoleCtrlHandler(eventHandler, TRUE);
#endif

    try
    {
        cxxopts::Options options("FFSERVER", "FF Server");
        options.add_options()
            ("e,exitcode", "the exit code for this program", cxxopts::value<int>(params.exitCode)->default_value("0"))
            ("l,loop-times", "what times will the program loop, zero or negative means loop forever", cxxopts::value<i32>(params.loopTimes)->default_value("5"))
            ("s,sleep-time", "time for sleep in seconds", cxxopts::value<int>(params.sleepTime)->default_value("1"))
            ("h,help", "print help")
            ;

        auto result = options.parse(argc, argv);
        if (result.count("help"))
        {
            std::cout << options.help();
            return 0;
        }
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (params.loopTimes <= 0)
    {
        loopForever();
    }
    else
    {
        loopLimited();
    }

    return params.exitCode;
}

static void loopLimited()
{
    for (i32 i = 0; i < params.loopTimes; ++i)
    {
        std::cout << "Hello world!" << std::endl;
        sleep(params.sleepTime);
    }
}

static void loopForever()
{
    while (1)
    {
        std::cout << "Hello world!" << std::endl;
        sleep(params.sleepTime);
    }
}

static void sighandler(int signum)
{
    std::cout << "Signaled: " << signum << std::endl;
    std::cout << "Good Bye!" << std::endl;
    exit(params.exitCode);
}

#ifdef _WIN32
static BOOL eventHandler(DWORD dwCtrlType)
{
    sighandler(dwCtrlType);
    return TRUE;
}
#endif
