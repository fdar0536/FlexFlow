/*
 * Flex Flow
 * Copyright (c) 2026-present fdar0536
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

#ifdef _WIN32
#include "windows.h"
#define sleep(x) Sleep(x * 1000)
#else
#include "unistd.h"
#endif

#include "CLI/CLI.hpp"

#include "model/defines.h"

static void sighandler(int signum);

#ifdef _WIN32
static BOOL eventHandler(DWORD dwCtrlType);
#endif

typedef struct Param
{
    u32 loop;
    u32 sleep;
    i32 exitCode;
} Param;

static Param param;

int main(int argc, char **argv)
{
    UNUSED(argc);
    signal(SIGABRT, sighandler);
    signal(SIGFPE,  sighandler);
    signal(SIGILL,  sighandler);
    signal(SIGINT,  sighandler);
    signal(SIGSEGV, sighandler);
    signal(SIGTERM, sighandler);

    CLI::App app{"Testing prog for Flex Flow", argv[0]};
    app.add_option("-l,--loop", param.loop, "How much time to loop, 0 means loop forever")->default_val(5);
    app.add_option("-s,--sleep", param.sleep, "How much time to sleep")->default_val(1);
    app.add_option("-e,--exit-code", param.exitCode, "set exit code for this program")->default_val(0);
    CLI11_PARSE(app, argc, argv);

    if (param.loop)
    {
        for (u32 i = 0; i < param.loop; ++i)
        {
            std::cout << "Hello world!" << std::endl;
            sleep(param.sleep);
        }
    }
    else
    {
        while (true)
        {
            std::cout << "Hello world!" << std::endl;
            sleep(param.sleep);
        }
    }

    return param.exitCode;
}

static void sighandler(int signum)
{
    UNUSED(signum);
    exit(param.exitCode);
}

#ifdef _WIN32
static BOOL eventHandler(DWORD dwCtrlType)
{
    sighandler(dwCtrlType);
    return TRUE;
}
#endif
