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

#ifndef _MODEL_PROC_WINPROC_HPP_
#define _MODEL_PROC_WINPROC_HPP_

#include <atomic>
#include <deque>
#include <mutex>
#include <thread>

#include "windows.h"

#if WINVER < 0x0A00
#error "windows 10 or later only"
#endif

#include "iproc.hpp"

namespace Model
{

namespace Proc
{

class WinProc : public IProc
{
public:

    WinProc();

    ~WinProc();

    virtual u8 init() override;

    virtual u8 start(const Task &task) override;

    virtual void stop() override;

    virtual bool isRunning() override;

    virtual void readCurrentOutput(std::vector<std::string> &out) override;

    virtual u8 exitCode(i32 &out) override;

private:

    HANDLE m_childStdoutRead = nullptr;

    HANDLE m_childStdoutWrite = nullptr;

    HANDLE m_childStdinRead = nullptr;

    HANDLE m_childStdinWrite = nullptr;

    PROCESS_INFORMATION m_procInfo;

    std::atomic<i32> m_exitCode;

    // pseudo console
    HPCON m_pseudoConsole = nullptr;

    u8 prepareStartupInformation(STARTUPINFOEX *output);

    u8 CreateChildProcess(const Task &);

    void resetHandle();

    void stopImpl();

    std::jthread m_thread;

    std::mutex m_mutex;

    std::deque<std::string> m_deque;

    void readOutputLoop();

};

} // end namespace Proc

} // end namespace Model

#endif // _MODEL_PROC_WINPROC_HPP_
