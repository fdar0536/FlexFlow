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

#include "util.h"
#include "spdlog/spdlog.h"
#include "controller/global/global.hpp"

#include "macproc.hpp"

namespace Model
{

namespace Proc
{

MacProc::MacProc():
    PosixProc()
{}

MacProc::~MacProc()
{}

u8 MacProc::start(const Task &task)
{
    if (isRunning())
    {
        spdlog::error("{}:{} Process is running", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (Controller::Global::isAdmin())
    {
        spdlog::error("{}:{} Refuse to run as super user", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    m_masterFD = -1;
    m_exitCode.store(0, std::memory_order_relaxed);

    m_pid = forkpty(&m_masterFD, NULL, NULL, NULL);
    if (m_pid == -1)
    {
        // parent process
        spdlog::error("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
        return 1;
    }

    if (m_pid == 0)
    {
        // child process
        startChild(task);
    }

    return 0;
}

bool MacProc::isRunning()
{
    return false;
}

} // end namespace Proc

} // end namespace Model
